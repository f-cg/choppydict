#include "storage.h"
#include <ctype.h>
#include "defines.h"
#include "debug.h"
#include "utf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_cmd_result(char *cmd, char *storage, int size)
{
	FILE *fp;
	/* Open the command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		errorexit("Failed to run command");
	}
	/* Read the output a line at a time - output it. */
	int count = fread(storage, 1, size, fp);
	/* close */
	pclose(fp);
	return count;
}

static int voice(char *storage, int size)
{
	FILE *fp;
	/* Open the command for reading. */
	fp = popen("mpg123 -q -", "w");
	if (fp == NULL) {
		errorexit("Failed to run command");
	}
	int count = fwrite(storage, 1, size, fp);
	/* close */
	pclose(fp);
	return count;
}

void ws_print(const word_storage_t *ws)
{
	printf("basic_len:%d; meaningend-word:%ld\n", ws->basic_len,
	       ws->meaning - ws->word_m + strlen(ws->meaning));
	printf("word:%s,phonetic_symbol1:%s,phonetic_symbol2:%s,\nmeaning:%s, chart1:%s, chart2:%s\n",
	       ws->word_m, ws->phonetic_symbol1, ws->phonetic_symbol2,
	       ws->meaning, ws->chart1, ws->chart2);
	printf("sound1_len:%d sound2_len:%d\n", ws->sound1_len, ws->sound2_len);
}

char *strrstr(const char *str1, const char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return NULL;
	char *last = NULL;
	char *current = NULL;
	//找到第一个
	current = strstr(str1, str2);
	while (current != NULL) {
		last = current;
		current = strstr(current + 1, str2);
	}
	return last;
}

char *url_decode(char *input)
{
	int input_length = strlen(input);

	size_t output_length = (input_length + 1) * sizeof(char);
	char *working = input;
	/** malloc(output_length); */
	char *output = working;

	while (*input) {
		if (*input == '%') {
			char buffer[3] = { input[1], input[2], 0 };
			*working++ = strtol(buffer, NULL, 16);
			input += 3;
		} else {
			*working++ = *input++;
		}
	}

	*working = 0; // null terminate
	return output;
}
int utf_decode(char s[])
{
	if (!s[0] || !s[1]) {
		printf("%s\n", "too short");
		return 0;
	}
	char *p = s;
	int i, n = 0;
	while (*p) {
		if (*p == '\\' && *(p + 1) == 'u') {
			int status = sscanf(p + 2, "%x%n", &i, &n);
			if (status == 1) {
				int nbytes = to_utf8(i, s);
				s += nbytes;
				p += 2 + n;
			} else {
				printf("%s\n", "sscanf failed!");
				return 0;
			}
		} else {
			*s = *p;
			++p;
			++s;
		}
	}
	*s = '\0';
	return 0;
}

char *parse_chart_data(char *p, char **qq, int chart_type)
{
	if (!p)
		return NULL;
	p = strstr(p, "data=\"");
	if (!p)
		return NULL;
	p += sizeof("data=\"") - 1;
	char *q = strchr(p, '"');
	if (!q)
		return NULL;
	*q = '\0';
	*qq = q;
	char *chart_data = url_decode(p);
	utf_decode(chart_data);
	p = chart_data;
	char *e = chart_data;
	char *percent, *sense;
	const char *part2 = chart_type == 1 ? "\"sense\":\"" : "\"pos\":\"";
	int part_len = strlen(part2);
	while ((percent = strstr(p, "\"percent\":"))) {
		percent += sizeof("\"percent\":") - 1;
		while (((*(e++)) = *(percent++)) != ',')
			;
		sense = strstr(p, part2) + part_len;
		*(e - 1) = ':';
		while (((*(e++)) = *(sense++)) != '"')
			;
		*(e - 1) = ',';
		p = sense > percent ? sense : percent;
	}
	*(e - 1) = '\0';
	return chart_data;
}

char *extract_meaning(char *meaning)
{
	char *s, *p, *q;
	p = q = s = meaning;
	while ((p = strstr(p, "<span>"))) {
		p += sizeof("<span>") - 1;
		q = strstr(p, "</span>");
		if (!q)
			break;
		while (p != q)
			*(s++) = *(p++);
		*(s++) = ':';
		p = strstr(p, "<strong>");

		p += sizeof("<strong>") - 1;
		q = strstr(p, "</strong>");
		if (!q)
			break;
		while (p != q)
			*(s++) = *(p++);
		*(s++) = '\n';
	}
	*(s - 1) = '\0';
	return meaning;
}

word_storage_t *net_get(char *word)
{
	char curl[1024] = GET DICTCN;
	strcat(curl, word);
	const int size = 64 * 1024; //64*8K
	char storage[STORAGE_SIZE] = { [0] = 0 };
	get_cmd_result(curl, storage, size);
	if (!storage[0]) {
		DEBUG_ERR("get zero bytes");
		return NULL;
	}
	char *p, *q;
	// get phonetic_symbols
	p = strstr(storage, "<div class=\"phonetic\">");
	if (!p) {
		DEBUG_WARN("phonetic_symbols not found!");
		return NULL;
	}
	const char bdo[] = "<bdo lang=\"EN-US\">";
	p = strstr(p, bdo);
	if (!p) {
		DEBUG_WARN(
			"phonetic_symbols not found: <bdo lang=...> not found");
		return NULL;
	}
	q = strstr(p, "</bdo>");
	if (!q) {
		DEBUG_WARN("phonetic_symbols not found: </bdo> not found");
		return NULL;
	}
	p += sizeof(bdo);
	*(q - 1) = '\0';
	char *phonetic_symbol1 = p;
	char *phonetic_symbol2 = q - 1; //default point to '\0'
	p = strstr(q, bdo);
	if (!p) {
		DEBUG_WARN("phonetic_symbols not found: </bdo> not found");
		return NULL;
	}
	q = strstr(p, "</bdo>");
	if (q) {
		p += sizeof(bdo);
		*(q - 1) = '\0';
		phonetic_symbol2 = p;
	}
	// get word meaning
	p = strstr(q, "<ul class=\"dict-basic-ul\">");
	q = strstr(q, "<div class=\"dict-chart\"");
	if (!p || !q) {
		DEBUG_WARN("no meaning");
		return NULL;
	}
	*(q - 1) = '\0';
	*(strrstr(p, "</strong>") + sizeof("</strong>")) = '\0';
	char *meaning = extract_meaning(p);
	printf("p:%p\n", p);
	printf("q:%p\n", q);

	// get word chart data
	p = strstr(q, "id=\"dict-chart-basic\"");
	char *chart1, *chart2;
	printf("p:%p\n", p);
	if (p) {
		p += sizeof("id=\"dict-chart-basic\"");
		chart1 = parse_chart_data(p, &q, 1);
	} else {
		chart1 = q;
		*(q) = '\0';
	}
	p = q + 1;
	p = strstr(q + 1, "id=\"dict-chart-examples\"");
	if (p) {
		p += sizeof("id=\"dict-chart-examples\"");
		chart2 = parse_chart_data(p, &q, 2);
	} else {
		chart2 = q;
		*(q) = '\0';
	}

	// get ws length
	int word_len = strlen(word);
	int phonetic_symbol1_len = strlen(phonetic_symbol1);
	int phonetic_symbol2_len = strlen(phonetic_symbol2);
	int meaning_len = strlen(meaning);
	int chart1_len = strlen(chart1);
	int chart2_len = strlen(chart2);
	int basic_len = word_len + phonetic_symbol1_len + phonetic_symbol2_len +
			meaning_len + chart1_len + chart2_len + 6;
	word_storage_t *ws = malloc(sizeof(word_storage_t));
	// set basic
	ws->basic_len = basic_len;
	ws->word_m = malloc(basic_len);
	// copy word
	strcpy(ws->word_m, word);
	// copy phonetic_symbols
	ws->phonetic_symbol1 = ws->word_m + word_len + 1;
	strcpy(ws->phonetic_symbol1, phonetic_symbol1);
	ws->phonetic_symbol2 = ws->phonetic_symbol1 + phonetic_symbol1_len + 1;
	strcpy(ws->phonetic_symbol2, phonetic_symbol2);
	// copy meaning
	ws->meaning = ws->phonetic_symbol2 + phonetic_symbol2_len + 1;
	strcpy(ws->meaning, meaning);
	// copy chart
	ws->chart1 = ws->meaning + meaning_len + 1;
	strcpy(ws->chart1, chart1);
	ws->chart2 = ws->chart1 + chart1_len + 1;
	strcpy(ws->chart2, chart2);

	//get sound
	strcpy(curl,
	       GET "https://ssl.gstatic.com/dictionary/static/sounds/oxford/");
	strcat(curl, word);
	int curllen = strlen(curl);

	strcpy(curl + curllen, "--_gb_1.mp3");
	int soundlen = get_cmd_result(curl, storage, size);
	ws->sound1_len = soundlen;
	ws->sound1_m = malloc(soundlen);
	memcpy(ws->sound1_m, storage, soundlen);

	strcpy(curl + curllen, "--_us_1.mp3");
	soundlen = get_cmd_result(curl, storage, size);
	ws->sound2_len = soundlen;
	ws->sound2_m = malloc(soundlen);
	memcpy(ws->sound2_m, storage, soundlen);

	//save to cache
	save(ws);

	return ws;
}

void destroy(word_storage_t *ws)
{
	if (!ws)
		return;
	free(ws->word_m);
	free(ws->sound1_m);
	free(ws->sound2_m);
	free(ws);
}
word_storage_t *get_ws(char *word)
{
	word_storage_t *ws = get_cached(word);
	if (ws)
		return ws;
	return net_get(word);
}

char *get_selected_word()
{
	char *word = calloc(WL, sizeof(word[0]));
	int count = get_cmd_result("xclip -out", word, WL - 1);
	if (!count) {
		free(word);
		return NULL;
	}
	int len = strlen(word);
	char *b, *e;
	b = word;
	while (*b && !isalpha(*b))
		++b;
	e = b;
	b = word;
	while (*e && isalpha(*e))
		*(b++) = *(e++);
	*b = '\0';
	DEBUG_INFO("word:%s", word);
	if (strlen(word)) {
		return word;
	}
	free(word);
	return NULL;
}

void ws_voice(word_storage_t *ws)
{
	voice(ws->sound1_m, ws->sound1_len);
	voice(ws->sound2_m, ws->sound2_len);
	FILE *fp = fopen("a.mp3", "w");
	fwrite(ws->sound1_m, ws->sound1_len, 1, fp);
	fclose(fp);
}

void ws_desc(word_storage_t *ws)
{
	ws_print(ws);
	ws_voice(ws);
}
