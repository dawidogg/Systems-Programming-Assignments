#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

typedef enum {
    EXACT, PREFIX
} MatchStrategy;

typedef struct {
    int size;
    char** string;
} StringList;

typedef struct {
    MatchStrategy strat;
    StringList *list;
} GetChunkOptions;

#define ALLOC_SIZE(n) ((1 << (32 - __builtin_clz(n)))+1)

void StringList_init(StringList *sl) {
    sl->size = 0;
    sl->string = malloc(sizeof(size_t));
}

void StringList_push(StringList *sl, const char *s, size_t s_len) {
    if (ALLOC_SIZE(sl->size+1) > ALLOC_SIZE(sl->size)) {
        sl->string = realloc(sl->string, sizeof(char*)*ALLOC_SIZE(sl->size+1));
    }
    sl->string[sl->size] = malloc(sizeof(char)*(s_len+1));
    memcpy(sl->string[sl->size], s, s_len);
    sl->string[sl->size][s_len] = '\0';
    sl->string[sl->size+1] = NULL;
    sl->size++;
}

void StringList_free(StringList *sl) {
    for (int i = 0; i < sl->size; i++) {
        if (sl->string[i] != NULL)
            free(sl->string[i]);
    }
}

void StringList_view(StringList *sl) {
    for (int i = 0; i < sl->size; i++) {
        printf("%s\n", sl->string[i]);
    }
}

void StringList_remove_trailing_dot(StringList *sl) {
    if (sl->string[sl->size-1][0] == 46) {
        free(sl->string[sl->size-1]);
        sl->size--;
    }
}

size_t get_chunk(void *ptr, size_t size, size_t nmemb, GetChunkOptions *opt) {
    char *token = strtok(ptr, "\n");
    while (token != NULL) {
        int status_code = 0;
        sscanf(token, "%d", &status_code);
        int skip_first = 0;
        int skip_last = 0;
        if (status_code == 150) skip_first = 3;
        if (opt->strat == PREFIX) {
            char *temp = strstr(token, "\"");
            if (temp != NULL) {
                token = temp+1;
                skip_first = temp-token+1;
                skip_last = 2;
            }
        }
        int ignored_codes[] = {151, 152, 220, 221, 250};
        int ignored_codes_len = sizeof(ignored_codes)/sizeof(ignored_codes[0]);
        int ignore_line = 0;
        for (int i = 0; i < ignored_codes_len && !ignore_line; i++)
            ignore_line |= (status_code == ignored_codes[i]);
        if (!ignore_line)
            StringList_push(opt->list, token+skip_first*sizeof(char), strlen(token)-skip_last*sizeof(char));
        token = strtok(NULL, "\n");
    }
    return size*nmemb;
}

const char server[] = "127.0.0.1";
// const char server[] = "dict.org";
// const char database[] = "*";
const char database[] = "devil";

void create_link(char *dest, const char *to_search, MatchStrategy strat) {
    switch (strat) {
    case EXACT:
        sprintf(dest, "dict://%s/d:%s:%s", server, to_search, database);
        break;
    case PREFIX:
        sprintf(dest, "dict://%s/m:%s:%s:prefix", server, to_search, database);
        break;
    }
}

void dictionary_fetch(StringList *dest, const char *word, MatchStrategy strat) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) return;

    GetChunkOptions opt = {strat, dest};
    char link[256];
    create_link(link, word, strat);
    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &opt);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    StringList_remove_trailing_dot(dest);
    curl_easy_cleanup(curl);
}

int main(int argc, char *argv[]) {
    for (char letter[] = "a"; letter[0] <= 'z'; letter[0]++) {
        StringList index;
        StringList_init(&index);
        printf("Index for letter %s:\n", letter);
        dictionary_fetch(&index, letter, PREFIX);
        StringList_view(&index);
        printf("------------------------------------------------------------\n");
        StringList_free(&index);
    }
    return 0;
}
