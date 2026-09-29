#include <stdio.h>
#include <string.h>

int XOR(int a, int b) {
    if (a == b)
        return 0;
    else
        return 1;
}

int main() {
    int choice;
    int hop[] = {1, 3, 2, 0};
    int n = 4;

    printf("===== FHSS Program =====\n");
    printf("1. Sender\n");
    printf("2. Receiver\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    // SENDER
    if (choice == 1) {
        char data[100];

        printf("Enter binary data: ");
        scanf("%s", data);

        printf("FHSS Channels: ");

        for (int i = 0; i < strlen(data); i++) {

            int bit = data[i] - '0';

            // XOR
            int encoded = XOR(bit, hop[i % n] % 2);

            // Calculate channel
            int channel = (hop[i % n] + encoded) % 4;

            printf("%d ", channel);
        }

        printf("\n");
    }

    // RECEIVER
    else if (choice == 2) {
        int channel[100];
        int count;

        printf("Enter number of channels: ");
        scanf("%d", &count);

        printf("Enter FHSS channels: ");

        for (int i = 0; i < count; i++)
            scanf("%d", &channel[i]);

        printf("Decoded Data: ");

        for (int i = 0; i < count; i++) {

            // Get encoded bit
            int encoded = (channel[i] - hop[i % n] + 4) % 4;

            // XOR
            int bit = XOR(encoded % 2, hop[i % n] % 2);

            printf("%d", bit);
        }

        printf("\n");
    }

    else {
        printf("Invalid choice!\n");
    }

    return 0;
}