#include "get_word.h"

int main(void)
{
/**     [> word_storage_t *ws = get_word("hello"); <] */
	word_storage_t *ws = get_word("record");
	ws_desc(ws);
	destroy(ws);
	return 0;
}
