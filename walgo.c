/*
 * Copyright (C) 2025 Davide Usberti <usbertibox@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * ======================= INFORMATION ABOUT THE PROGRAM =======================
 * This simple game is inspired by "Words of Wonders", a word puzzle where
 * the goal is to guess valid words from a set of given letters.
 *
 * + Rules:
 *      + Words:
 *        1. Each word must be at least 3 characters long.
 *        2. Words can only contain ASCII uppercase letters (A–Z, 65–90).
 *
 *      + Characters:
 *        1. The set of available characters contains 5 letters.
 *
 * + Algorithm:
 *      1. Parse and create an array of valid words.
 *      2. Choose a random word from the array as the "master word".
 *      3. Select 8 words that can be formed using only letters
 *         from the master word.
 *      4. Shuffle and display the master word to the user.
 *      5. Wait for user input.
 *      6. Check if the guessed word is valid and assign points.
 *      7. End.
 *
 * ================================= DISCLAIMER ================================
 * This project was created by me (Davide Usberti) after a brief reflection on
 * the algorithm behind the game "Words of Wonders". I was on a train playing WOW,
 * and after a few minutes I thought about recreating it in C. Once I got home,
 * I immediately started coding.
 *
 * This software is heavily commented because I believe it should be readable
 * and educational for students and beginners. The techniques used here may not
 * be the most efficient, but they are extremely minimal and free of unnecessary
 * complexity.
 *
 * All utilities that don't need external dependencies are implemented directly
 * within the source code. Adding external libraries would make no sense for a
 * project like this.
 *
 * Code is Art.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LOG(str, ...) printf("LOG: " str "\n", ##__VA_ARGS__);
#define ERR(str, ...) fprintf(stderr, "ERR: " str "\n", ##__VA_ARGS__);

/* Include the IT word list */
#include "words.it.def"
/* Include the EN word list */
#include "words.en.def"

/* Rapresentation of the word with its
 * basic information and len */
typedef struct {
      const char *text; /* Uppercase word */
      size_t w_len; /* word length */
} word_t;

/* This is a wrapper for words inserted into
 * the linked list as a node. */
typedef struct word_node_t {
        word_t word; /* effective struct of the word */
        int found; /* true if the word is already guessed */
        int hint_index; /* index of char to show as hint */
        struct word_node_t *next; /* next word node */
} word_node_t;

/* This structure is made to store some information
 * aobut the whole list of words. Also the head, the
 * starting point to iterate it. */
typedef struct {
        struct word_node_t *head;
        size_t size;
} wordlist_t;

/* protypes */

/* Check if the word is able to be converted and parsed */
int isValidWord(const char *word) {
        size_t len = strlen(word);

        /* Check the length (at least 3 chars and max 5 chars) */
        if (len < 3 || len > 5) return 0;

        /* Check if chars are valid (uppercase without accents) */
        for (size_t i = 0; word[i] != '\0'; i++) {
                char c = word[i];
                if (c < 65 || c > 90) return 0;
        }

        return 1;
}

/* Provide the length of a char ** with unknown size */
static inline size_t getWordlistLen(const char **wordlist) {
        size_t size = 0;
        while (wordlist[size] != NULL)
                size++;

        return size;
}

/* Create a new dynamic node that contains the word_t struct */
word_node_t *createNode(word_t word) {
        word_node_t *new_node = malloc(sizeof(word_node_t));

        if (!new_node) {
               ERR("Failed to allocate word: %s", word.text);
               exit(EXIT_FAILURE);
        }

        new_node->word = word;
        new_node->found = 0;
        new_node->hint_index = rand() % word.w_len;
        new_node->next = NULL;
        return new_node;
}

/* By given node append it to the linked list */
void appendNode(wordlist_t *wl, word_node_t *node) {

        /* for empty list */
        if (wl->head == NULL) {
                wl->head = node;
        } else {

                /* append */
                word_node_t *tmp = wl->head;
                while (tmp->next)
                        tmp = tmp->next;
                tmp->next = node;
        }
        wl->size++;
}

/* Cleanup wordlist memory */
void releaseWordlist(wordlist_t *wl) {
        word_node_t *tmp;
        while (wl->head) {
                tmp = wl->head;
                wl->head = wl->head->next;
                free((void *)tmp->word.text);
                free(tmp);
        }
        wl->size = 0;
}

/* Uppercase a string pointer */
static inline void str_to_upper(char *s) {
    for (; *s; s++)
        if (*s >= 'a' && *s <= 'z')
            *s -= 32;
}

/* Convert the raw list (string only) to a linked list.
 * Returns: the szie of the list. */
size_t parseWords(wordlist_t *wl, const char **raw_list) {
        size_t total = getWordlistLen(raw_list);

        for (size_t i = 0; i < total; i++) {
                const char *w = raw_list[i];

                /* Uppercase the word */
                char *w_clone = strdup(w);
                str_to_upper(w_clone);

                if (!isValidWord(w_clone)) {
                        free(w_clone);
                        continue;
                }

                word_t word = { .text = w_clone, .w_len = strlen(w_clone) };
                word_node_t *node = createNode(word);
                appendNode(wl, node);
        }

        return wl->size;
}

/* Generate random number in range (included)  */
int randomRange(int min, int max) {
        return (min + rand() % (max - min + 1));
}

/* This function when is called generate a random number
 * between 0 and MAX_LIST_ITEM, iterates the linked list
 * and finds the right word. This also check if the word
 * is 5 characters long. */
word_t getMasterWord(wordlist_t *wl) {
        word_node_t *current = wl->head;

        if (wl->size == 0) {
                ERR("Wordlist is empty!");
                exit(EXIT_FAILURE);
        }

        /* Get a word with length = 5 */
        do {
                current = wl->head;
                /* Exctract the master word */
                int masterIndex = randomRange(0, (int) wl->size - 1);

                /* Browse array and find the word at the right index */
                for (int i = 0; i < masterIndex && current->next; i++)
                        current = current->next;
        } while (current->word.w_len != 5);

        return current->word;
}

/* Check if a raw word (char *) is contained
 * into a wordlist_t structured linked list. */
int isRawWordInWordlist(wordlist_t *wl, const char *raw_w) {
        word_node_t *current = wl->head;
        while (current != NULL) {
                if (strcmp(current->word.text, raw_w) == 0)
                        return 1;
                current = current->next;
        }

        return 0;
}

/* Check if a word given is contained into a
 * linked list of words. */
int isWordInWordlist(wordlist_t *wl, word_t *word) {
        return isRawWordInWordlist(wl, word->text);
}

/* Helper to swap two vars */
void swap(char *a, char *b) {
        char tmp = *a;
        *a = *b;
        *b = tmp;
}

/* This helper uses the iterative version of the
 * Heap's Algorithm to find all the letter combinations
 * by a given word.*/
char **getPermutations(const char *word, int *out_count) {
        int n = strlen(word);
        int total = 5*5*5 + 5*5*5*5 + 5*5*5*5*5;
        char **combinations = malloc(total * sizeof(char *));
        int comb_i = 0;

        for (int len = 3; len <= 5; len++) {
                int indices[5] = {0};
                int i;

                while (1) {
                        combinations[comb_i] = malloc((len + 1) * sizeof(char));
                        for (i = 0; i < len; i++)
                                combinations[comb_i][i] = word[indices[i]];
                        combinations[comb_i][len] = '\0';
                        comb_i++;

                        /* increment */
                        for (i = len - 1; i >= 0; i--) {
                                if (indices[i] < n - 1) {
                                        indices[i]++;
                                        break;
                                } else {
                                        indices[i] = 0;
                                }
                        }
                        if (i < 0) break;
                }
        }

        *out_count = comb_i;
        return combinations;
}

/* Free up memory used in the function `getPermutations(char *, int *)` */
void releasePermutations(char **combs, int *count) {
        for(int i = 0; i < *count; i++){
                free(combs[i]);
        }
        free(combs);
        *count = 0;
}

/* check if a word is already present in a wordlist */
int isWordAlreadyInList(wordlist_t *wl, const char *word) {
        word_node_t *current = wl->head;
        while (current != NULL) {
                if (strcmp(current->word.text, word) == 0)
                        return 1;
                current = current->next;
        }
        return 0;
}

int canFormWord(const char *word, const char *letters) {
        int letter_count[26] = {0};
        int word_count[26] = {0};
        
        for (int i = 0; letters[i] != '\0'; i++)
                letter_count[letters[i] - 'A']++;
        
        for (int i = 0; word[i] != '\0'; i++)
                word_count[word[i] - 'A']++;
        
        for (int i = 0; i < 26; i++)
                if (word_count[i] > letter_count[i])
                        return 0;
        
        return 1;
}

/* This function simply generate all the combinations
 * with the masterWords letters. For each combintation
 * checks if the word is contained in the dictionary.
 * Returns: linked list of words */
wordlist_t *getWords(wordlist_t *wl, word_t *masterWord) {
        wordlist_t *wl_generated = malloc(sizeof(wordlist_t));
        wl_generated->head = NULL;
        wl_generated->size = 0;

        /* Get permutations */
        int pcount;
        char **res = getPermutations(masterWord->text, &pcount);

        /* Parse words present into the dictionary */
        for (int i = 0; i < pcount; i++) {
               if (isRawWordInWordlist(wl, res[i]) 
                               && !isWordAlreadyInList(wl_generated, res[i])) {                       
                        char *word_copy = strdup(res[i]);
                        word_node_t *new_node = 
                                createNode((word_t){ .text = word_copy, .w_len = strlen(word_copy) });
                        /* Insert node */
                        appendNode(wl_generated, new_node);
               }
        }
        
        releasePermutations(res, &pcount);
        return wl_generated;
}

/* This function prints all the word with
 * just one unhidden char. 
 * Format:
 *      C _ _
 *      _ _ U _ _
 */
void printPlaceholder(const char *w, int found, int hint_index) {
        int n = strlen(w);
        
        if (found) {
                /* Show the whole word if found */
                for (int i = 0; i < n; i++) {
                        printf("%c ", w[i]);
                }
                printf(" ✓\n");
                return;
        }

        char s[n + 1];
        memset(s, '_', n);
        s[n] = '\0';

        s[hint_index] = w[hint_index];

        for (int i = 0; i < n; i++) {
                printf("%c ", s[i]);
        }
        printf("\n");
}

/* Simple utility function to shuffle
 * string characters. */
void shuffle_chars(char *arr, int n) {
        for (int i = n - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                char tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
        }
}

/* Function that count all the words found */
int countFoundWords(wordlist_t *wl) {
        int count = 0;
        word_node_t *c = wl->head;
        while (c != NULL) {
                if (c->found) count++;
                c = c->next;
        }
        return count;
}

int main(int argc, char **argv) {

        /* Default language: en */
        char *lan = "en";

        /* Check for language setting */
        if (argc > 1) {
                if (strcmp(argv[1], "en") == 0) {
                        lan = "en";
                        LOG("Language set to English");
                } else if (strcmp(argv[1], "it") == 0) {
                        lan = "it";
                        LOG("Language set to Italian");
                } else {
                        ERR("Failled to validate argv 1");
                        exit(EXIT_FAILURE);
                }
        }

        /* Seed generation */
        srand(time(NULL));

        /* Wordlist initialization for dictionary */
        wordlist_t wl = {0};

        /* Dictionary parsing */
        LOG("Parsing words...");
        size_t parsed = parseWords(&wl, 
                        strcmp(lan, "it") == 0 ? raw_it_wordlist : raw_en_wordlist);
        LOG("Parsed %zu valid words", parsed);

        /* Get the master word */
        word_t masterWord = getMasterWord(&wl);
        printf("Master word: %s\n", masterWord.text);

        /* Get words to guess */
        wordlist_t *wtg = getWords(&wl, &masterWord);

        char input[100];
        int score = 0;
        
        /* Start the game */
        while (1) {
                system("clear");
                
                printf("========================================================\n");
                printf("Score: %d\n\n", score);

                /* count found words */
                int found_count = countFoundWords(wtg);
                int total_count = wtg->size;
                
                /* Print placeholders */
                word_node_t *c = wtg->head;
                while (c != NULL) {
                        printf("        Word: ");
                        printPlaceholder(c->word.text, c->found, c->hint_index); 
                        c = c->next;
                }
                printf("\n");
                
                char letters[10];
                strncpy(letters, masterWord.text, masterWord.w_len);
                letters[masterWord.w_len] = '\0';
                shuffle_chars(letters, masterWord.w_len);

                /* Print letters avaible */
                printf("Letters: ");
                for (int i = 0; i < strlen(letters); i++)
                        printf("%c ", letters[i]);
                putchar('\n');
                
                /* if user found all the words */
                if (found_count == total_count) {
                        printf("\n * CONGRATULATIONS! You found all the words!\n");
                        printf("Final score: %d\n", score);
                        break;
                }


                printf("\nTry a word (or 'quit' to exit): ");

                /* get user input */
                if (scanf("%99s", input) != 1) {
                        ERR("Input error");
                        continue;
                }
                
                str_to_upper(input);
                
                /* quit input */
                if (strcmp(input, "QUIT") == 0) {
                        printf("Thank you to have played. Bye...\n");
                        break;
                }

                if (!canFormWord(input, masterWord.text)) {
                        printf("Word cannot be formed with available letters!\n");
                        sleep(1);
                        continue;
                }

                /* check if the word is valid */
                word_node_t *curr = wtg->head;
                int valid = 0;

                while (curr != NULL) {
                        /* word is correct */
                        if (strcmp(curr->word.text, input) == 0) {
                                /* check if the word is already found */
                                if (curr->found) {
                                        printf("You already found this word!\n");
                                } else {
                                        curr->found = 1;
                                        score += curr->word.w_len * 10;
                                        printf("Valid word! +%d points\n", (int)curr->word.w_len * 10);
                               
                                }
                                valid = 1;
                                break;
                        }
                        curr = curr->next;
                }

                if (!valid) 
                        printf("Word not valid!!\n");

                /* wait 1 seconds */
                sleep(1);
        }
        
        /* cleanup all */
        releaseWordlist(wtg);
        free(wtg);
        releaseWordlist(&wl);
        return 0;
}
