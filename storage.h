#ifndef STORAGE_H
#define STORAGE_H

typedef struct {
  int percent;
  char meaning[32];
} meaning_frequent_t;

typedef struct {
  int basic_len;
  char *word_m;
  char *phonetic_symbol1;
  char *phonetic_symbol2;
  char *meaning;
  char *chart1;
  char *chart2;
  int sound1_len;
  char *sound1_m;
  int sound2_len;
  char *sound2_m;
} word_storage_t;

int save(word_storage_t *ws);
word_storage_t* get_cached(const char *word);
/*
 * len word 0 phonetic_symbol1 0 meaning 0
 * len word 0 phonetic_symbol1 0 phonetic_symbol2 0 meaning 0
 * basic_len word 0 phonetic_symbol1 0 phonetic_symbol2 0 meaning 0 sound1_len sound1 sound2_len sound2 mchart_len=0
 * basic_len word 0 phonetic_symbol1 0 phonetic_symbol2 0 meaning 0 sound1_len sound1 sound2_len sound2 mchart_len mchart0 mchart1...
 *
 */
#endif
