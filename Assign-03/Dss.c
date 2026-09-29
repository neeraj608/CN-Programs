#include <stdio.h>
#include <string.h>

#define KEY "1011011000"

void sender() {
    char data[100];
    char ans[1000]; 
    int ptr = 0;

    printf("\nEnter Binary Data: ");
    scanf("%s", data);

    int dataLen = strlen(data);
    int keyLen = strlen(KEY);

    for (int i = 0; i < dataLen; i++) {
        for (int j = 0; j < keyLen; j++) {
            ans[ptr++] = ((data[i] - '0') ^ (KEY[j] - '0')) + '0';
        }
    }
    ans[ptr] = '\0';

    printf("\nTransmitted (Spread) Data: %s\n", ans);
}

void receiver() {
    char data[1000];

    printf("\nEnter Received Spread Data: ");
    scanf("%s", data);

    int dataLen = strlen(data);
    int keyLen = strlen(KEY);

    
    if (dataLen % keyLen != 0) {
        printf("\nError: Corrupted data (length mismatch).\n");
        return;
    }

    printf("\nOriginal Data: ");
    
    for (int i = 0; i < dataLen; i += keyLen) {
        
        char origBit = ((data[i] - '0') ^ (KEY[0] - '0')) + '0';
        printf("%c", origBit);
    }
    printf("\n");
}

int main() {
    int choice;

    printf("===== DSSS Sender / Receiver =====\n");
    printf("1. Sender (Spread)\n");
    printf("2. Receiver (Despread)\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            sender();
            break;
        case 2:
            receiver();
            break;
        default:
            printf("Invalid Choice!\n");
    }

    return 0;
}