#include <switch.h>

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER "192.168.1.140"
#define PORT 8192

u8 data[5];
JoystickPosition joystickLeft, joystickRight;


int main(int argc, char* argv[]) {
    socketInitializeDefault();
    consoleInit(NULL);

    printf("|------------------------|\n");
    printf("| Switch XBOX Controller |\n");
    printf("|------------------------|\n\n");

    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("socket %d\n", s);
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
    }

    while (appletMainLoop()) {
        hidScanInput();
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
        hidJoystickRead(&joystickLeft, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
        hidJoystickRead(&joystickRight, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);

        if (kHeld & KEY_DUP) sendto(s, "\x1\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x1\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_DDOWN) sendto(s, "\x2\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x2\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_DLEFT) sendto(s, "\x3\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x3\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_DRIGHT) sendto(s, "\x4\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x4\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_MINUS) sendto(s, "\x5\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x5\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_PLUS) sendto(s, "\x6\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x6\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_LSTICK) sendto(s, "\x7\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x7\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_RSTICK) sendto(s, "\x8\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x8\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_L) sendto(s, "\x9\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x9\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_R) sendto(s, "\xA\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xA\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_TOUCH) sendto(s, "\xB\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xB\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_A) sendto(s, "\xC\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xC\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_B) sendto(s, "\xD\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xD\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_X) sendto(s, "\xE\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xE\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_Y) sendto(s, "\xF\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xF\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_ZL) sendto(s, "\x10\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x10\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        if (kHeld & KEY_ZR) sendto(s, "\x11\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x11\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        data[0] = 0x12;
        data[1] = joystickLeft.dx >> 8;
        data[2] = joystickLeft.dx & 0xFF;
        data[3] = joystickLeft.dy >> 8;
        data[4] = joystickLeft.dy & 0xFF;
        sendto(s, data, 5 , 0 , (struct sockaddr *) &si_other, slen);

        data[0] = 0x13;
        data[1] = joystickRight.dx >> 8;
        data[2] = joystickRight.dx & 0xFF;
        data[3] = joystickRight.dy >> 8;
        data[4] = joystickRight.dy & 0xFF;
        sendto(s, data, 5 , 0 , (struct sockaddr *) &si_other, slen);

        consoleUpdate(NULL);
    }

    consoleExit(NULL);

    return 0;
}
