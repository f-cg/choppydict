#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "defines.h"
/** const char [] = "/home/lily/code/dict.cn/words_list"; */
/** const char cache_file[] = "/home/lily/code/dict.cn/words_cache"; */

int save(word_storage_t *ws)
{
	FILE *fp = NULL;
	//open
	fp = fopen(cache_file, "a");
	long pos = ftell(fp);
	//basic
	fwrite(&(ws->basic_len), sizeof(ws->basic_len), 1, fp);
	fputs(ws->word_m, fp);
	fputc(0, fp);
	fputs(ws->phonetic_symbol1, fp);
	fputc(0, fp);
	fputs(ws->phonetic_symbol2, fp);
	fputc(0, fp);
	fputs(ws->meaning, fp);
	fputc(0, fp);
	fputs(ws->chart1, fp);
	fputc(0, fp);
	fputs(ws->chart2, fp);
	fputc(0, fp);
	//sound
	fwrite(&(ws->sound1_len), sizeof(ws->sound1_len), 1, fp);
	fwrite(ws->sound1_m, 1, ws->sound1_len, fp);
	fwrite(&(ws->sound2_len), sizeof(ws->sound2_len), 1, fp);
	fwrite(ws->sound2_m, 1, ws->sound2_len, fp);
	//close
	fclose(fp);

	fp = fopen(words_file, "a");
	fprintf(fp, "%s %ld\n", ws->word_m, pos);
	fclose(fp);

	return 1;
}

void align_ws(word_storage_t *ws)
{
	ws->phonetic_symbol1 = NEXT(ws->word_m);
	ws->phonetic_symbol2 = NEXT(ws->phonetic_symbol1);
	ws->meaning = NEXT(ws->phonetic_symbol2);
	ws->chart1 = NEXT(ws->meaning);
	ws->chart2 = NEXT(ws->chart1);
}

word_storage_t *get_cached(const char *word)
{
	FILE *fp = NULL;
	fp = fopen(words_file, "r");
	if (!fp)
		return NULL;
	const int wl = 36;
	char w[wl];
	int pos;
	while (fscanf(fp, "%s %d", w, &pos) != EOF) {
		if (!strcmp(w, word)) {
			fclose(fp);
			word_storage_t *ws = calloc(1, sizeof(word_storage_t));
			fp = fopen(cache_file, "r");
			if (!fp)
				return NULL;
			fseek(fp, pos, SEEK_SET);
			//read basic_len
			fread(&ws->basic_len, sizeof(ws->basic_len), 1, fp);
			//read basic
			ws->word_m = malloc(ws->basic_len);
			fread(ws->word_m, ws->basic_len, 1, fp);
			align_ws(ws);
			//read sound1_len and sound1
			fread(&ws->sound1_len, sizeof(ws->sound1_len), 1, fp);
			ws->sound1_m = malloc(ws->sound1_len);
			fread(ws->sound1_m, ws->sound1_len, 1, fp);
			//read sound2_len and sound2
			fread(&ws->sound2_len, sizeof(ws->sound2_len), 1, fp);
			ws->sound2_m = malloc(ws->sound2_len);
			fread(ws->sound2_m, ws->sound2_len, 1, fp);
			fclose(fp);
			return ws;
		}
	}
	return NULL;
}
