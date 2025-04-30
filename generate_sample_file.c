#include <stdio.h>

int main(void) {
    FILE *fp = fopen("counting", "wb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    for (int i = 1; i <= 255; i++) {
        fputc(i, fp);
    }

    fclose(fp);
    return 0;
}