/*
CSV Transaction Analyzer

Input:
Take the filename as the first command-line argument.
Each line of the file is a CSV with three fields:

id,amount_cents,category


id is a single word (no commas), up to 24 chars.

amount_cents is a 32-bit signed integer (can be negative).

category is a single word (no commas), up to 24 chars.

Example lines:

T001,1299,food
T002,-500,refund
T003,2500,transport
T004,1299,food


Tasks:

Count — total number of valid transactions read.

Sum — total of amount_cents across all transactions.

Average — average amount (in dollars) to 2 decimals.

Min / Max — minimum and maximum amount_cents (print in dollars to 2 decimals).

Median — median of amount_cents across all transactions (print in dollars to 2 decimals).

If even count, median is average of the two middle values.

Top Category by Total — the category whose sum(amount_cents) is largest (print the category name and its total in dollars to 2 decimals).

Category Summary — for each category seen, print one line: Category Count Total($)

Categories should be printed sorted by name ascending.

Use a dynamically growing array for categories; aggregate as you go.

Format:
Labels left-justified in a 14-character field followed by =. Dollar amounts printed as dollars with two decimals (i.e., amount_cents / 100.0).

Example Output:

Count          = 4
Sum            = $46. - actually print $46.98; this is just a placeholder
Average        = $11.75
Min            = $-5.00
Max            = $25.00
Median         = $12.99
Top Category   = food ($25.98)

Category  Count  Total($)
food      2      25.98
refund    1      -5.00
transport 1      25.00


Requirements & Notes:

Skip malformed lines (e.g., missing fields, non-integer amount).

Use malloc/realloc for both the transactions array and the category table.

To compute Median, you must sort the amounts (you may use qsort here unless your instructor forbids it; if not allowed, implement insertion sort).

Convert cents to dollars only when printing (keep integers internally to avoid FP drift).

Empty file: print zeros/blank equivalents; no category table lines.

Suggested parsing: read a line with fgets, then strtok by ,, validate each field.

Hints the grader might expect:

Category aggregation can be a simple linear search over a dynamic array of {name, count, total_cents}; for small N that’s fine.

After reading, sort the category array by name (ascending) for the summary.

Be careful with formatting widths and %.2f for dollar outputs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_FIELD 24
#define LINE_BUF  256

typedef struct {
    char name[MAX_FIELD + 1];
    long long total_cents;
    int count;
} Category;

static int cmp_int_asc(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static int cmp_category_name_asc(const void *a, const void *b) {
    const Category *ca = (const Category *)a;
    const Category *cb = (const Category *)b;
    return strcmp(ca->name, cb->name);
}

static int find_category(Category *cats, int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (strcmp(cats[i].name, name) == 0) return i;
    }
    return -1;
}

static int parse_int(const char *s, int *out) {
    // Parse 32-bit signed integer with validation
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    if (v < -2147483648L || v > 2147483647L) return 0;
    *out = (int)v;
    return 1;
}

static void chomp(char *s) {
    size_t n = strcspn(s, "\r\n");
    s[n] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    // Dynamic arrays
    size_t amount_cap = 32, amount_n = 0;
    int *amounts = malloc(amount_cap * sizeof *amounts);
    if (!amounts) { perror("malloc"); fclose(f); return 1; }

    size_t cat_cap = 16, cat_n = 0;
    Category *cats = malloc(cat_cap * sizeof *cats);
    if (!cats) { perror("malloc"); free(amounts); fclose(f); return 1; }

    long long sum_cents = 0;
    int have_minmax = 0;
    int min_cents = 0, max_cents = 0;
    long long count = 0;

    char line[LINE_BUF];
    while (fgets(line, sizeof line, f)) {
        chomp(line);
        if (line[0] == '\0') continue;

        // Tokenize CSV: id,amount_cents,category
        char *id = strtok(line, ",");
        char *amount_str = strtok(NULL, ",");
        char *category = strtok(NULL, ",");

        // Validate presence
        if (!id || !amount_str || !category) continue;

        // Trim spaces around tokens if any (optional simple trim)
        while (*id == ' ' || *id == '\t') id++;
        while (*amount_str == ' ' || *amount_str == '\t') amount_str++;
        while (*category == ' ' || *category == '\t') category++;

        // Ensure fields are single words and within max length
        if (strlen(id) > MAX_FIELD || strlen(category) > MAX_FIELD) continue;

        int amount_cents;
        if (!parse_int(amount_str, &amount_cents)) continue;

        // Accumulate stats
        if (amount_n == amount_cap) {
            amount_cap *= 2;
            int *p = realloc(amounts, amount_cap * sizeof *amounts);
            if (!p) { perror("realloc"); free(cats); free(amounts); fclose(f); return 1; }
            amounts = p;
        }
        amounts[amount_n++] = amount_cents;

        sum_cents += amount_cents;
        count++;

        if (!have_minmax) {
            have_minmax = 1;
            min_cents = max_cents = amount_cents;
        } else {
            if (amount_cents < min_cents) min_cents = amount_cents;
            if (amount_cents > max_cents) max_cents = amount_cents;
        }

        // Update category aggregate
        int idx = find_category(cats, (int)cat_n, category);
        if (idx < 0) {
            if (cat_n == cat_cap) {
                cat_cap *= 2;
                Category *q = realloc(cats, cat_cap * sizeof *cats);
                if (!q) { perror("realloc"); free(cats); free(amounts); fclose(f); return 1; }
                cats = q;
            }
            strncpy(cats[cat_n].name, category, MAX_FIELD);
            cats[cat_n].name[MAX_FIELD] = '\0';
            cats[cat_n].total_cents = amount_cents;
            cats[cat_n].count = 1;
            cat_n++;
        } else {
            cats[idx].total_cents += amount_cents;
            cats[idx].count += 1;
        }
    }
    fclose(f);

    // Compute median
    double median_dollars = 0.0;
    if (amount_n > 0) {
        qsort(amounts, amount_n, sizeof *amounts, cmp_int_asc);
        if (amount_n % 2 == 1) {
            median_dollars = amounts[amount_n / 2] / 100.0;
        } else {
            int a = amounts[amount_n / 2 - 1];
            int b = amounts[amount_n / 2];
            median_dollars = ((double)a + (double)b) / 2.0 / 100.0;
        }
    }

    // Average
    double average_dollars = (count > 0) ? (sum_cents / 100.0) / (double)count : 0.0;

    // Top category by total_cents (break ties by name ascending)
    int top_idx = -1;
    if (cat_n > 0) {
        top_idx = 0;
        for (size_t i = 1; i < cat_n; i++) {
            if (cats[i].total_cents > cats[top_idx].total_cents) {
                top_idx = (int)i;
            } else if (cats[i].total_cents == cats[top_idx].total_cents) {
                if (strcmp(cats[i].name, cats[top_idx].name) < 0) {
                    top_idx = (int)i;
                }
            }
        }
    }

    // Sort categories by name ascending for summary table
    if (cat_n > 1) {
        qsort(cats, cat_n, sizeof *cats, cmp_category_name_asc);
    }

    // Output
    printf("%-14s = %lld\n", "Count", count);
    printf("%-14s = $%.2f\n", "Sum", sum_cents / 100.0);
    printf("%-14s = $%.2f\n", "Average", average_dollars);
    printf("%-14s = $%.2f\n", "Min", have_minmax ? (min_cents / 100.0) : 0.0);
    printf("%-14s = $%.2f\n", "Max", have_minmax ? (max_cents / 100.0) : 0.0);
    printf("%-14s = $%.2f\n", "Median", median_dollars);

    if (top_idx >= 0) {
        // Find current index of the original top category name after sort
        // Simpler: recompute on sorted array (same rule)
        int top2 = 0;
        for (size_t i = 1; i < cat_n; i++) {
            if (cats[i].total_cents > cats[top2].total_cents) top2 = (int)i;
            else if (cats[i].total_cents == cats[top2].total_cents &&
                     strcmp(cats[i].name, cats[top2].name) < 0) top2 = (int)i;
        }
        printf("%-14s = %s ($%.2f)\n", "Top Category",
               cats[top2].name, cats[top2].total_cents / 100.0);
    } else {
        printf("%-14s = \n", "Top Category");
    }

    printf("\nCategory  Count  Total($)\n");
    for (size_t i = 0; i < cat_n; i++) {
        printf("%-8s  %-5d  %7.2f\n",
               cats[i].name, cats[i].count, cats[i].total_cents / 100.0);
    }

    free(cats);
    free(amounts);
    return 0;
}
