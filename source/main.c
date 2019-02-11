#include <switch.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 8192
#define CPU_CLOCK 250000000 //250 MHz should be a BIG improvement in battery life if the console is used in handheld mode

char ipAddress[16];
u8 data[5];
JoystickPosition joystickLeft, joystickRight;

void * get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int broadcast(int sck, char * host, char * port) {
  uint8_t msg[] = "xbox_switch";
  char buffer[128] = {0};
  struct hostent * he;
  struct sockaddr_in remote;
  struct sockaddr_storage from;
  socklen_t from_len = sizeof(from);

  if ((he = gethostbyname(host)) == NULL) {
    return -1;
  }

  remote.sin_family = AF_INET;
  remote.sin_port = htons(atoi(port));
  remote.sin_addr = *(struct in_addr *)he->h_addr;
  memset(remote.sin_zero, 0, sizeof(remote.sin_zero));

  int on=1;
  setsockopt(sck, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000000;
  setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv));

  while(1) {
    while (1) {
      sendto(sck, msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
      recvfrom(sck, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);

      if (strlen(buffer) > 0) break;
    }

    char addr[40] = {0};

    const char * ptr = inet_ntop(from.ss_family, get_in_addr((struct sockaddr *)&from), addr, sizeof(addr));

    if (strcmp("xbox", buffer) == 0) {
      strcpy(ipAddress, ptr);
      break;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
	u8 ipBlock1 = 192;
	u8 ipBlock2 = 168;
    u8 ipBlock3 = 1;
    u8 ipBlock4 = 255;
    u8 currentIpBlock = 0;    

    socketInitializeDefault();
    consoleInit(NULL);

    printf("|------------------------|\n");
    printf("| Switch XBOX Controller |\n");
    printf("|------------------------|\n\n");

    printf("Please set up the ip you would like to send the UDP broadcast too\n.");
    printf("Usally this address is 192.168.X.255 where X is the 3rd block of your local IP.\n");
    printf("If UDP broadcasting does not work for you, use the IP of your computer instead.\n");

	consoleUpdate(NULL);

	pcvSetClockRate(PcvModule_Cpu, CPU_CLOCK); //Underclock the CPU
	appletSetScreenShotPermission(0); //Disable the screenshot function because it is not needed for the program
	appletBeginBlockingHomeButton(0); //Block the Home Button to prevent acidental use. Mainly useful for the pro controller since (+) and Home are close together.

    while (1)
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

		if (currentIpBlock == 0) printf("\x1b[11;1HCurrent IP Adress: [%d].%d.%d.%d\t\t\n", ipBlock1, ipBlock2, ipBlock3, ipBlock4);
		if (currentIpBlock == 1) printf("\x1b[11;1HCurrent IP Adress: %d.[%d].%d.%d\t\t\n", ipBlock1, ipBlock2, ipBlock3, ipBlock4);
		if (currentIpBlock == 2) printf("\x1b[11;1HCurrent IP Adress: %d.%d.[%d].%d\t\t\n", ipBlock1, ipBlock2, ipBlock3, ipBlock4);
		if (currentIpBlock == 3) printf("\x1b[11;1HCurrent IP Adress: %d.%d.%d.[%d]\t\t\n", ipBlock1, ipBlock2, ipBlock3, ipBlock4);

		//Handle inputs
		if (kDown & KEY_DUP && currentIpBlock == 0) ipBlock1++;
		if (kDown & KEY_DDOWN && currentIpBlock == 0) ipBlock1--;
		if (kDown & KEY_DUP && currentIpBlock == 1) ipBlock2++;
		if (kDown & KEY_DDOWN && currentIpBlock == 1) ipBlock2--;
		if (kDown & KEY_DUP && currentIpBlock == 2) ipBlock3++;
		if (kDown & KEY_DDOWN && currentIpBlock == 2) ipBlock3--;
		if (kDown & KEY_DUP && currentIpBlock == 3) ipBlock4++;
		if (kDown & KEY_DDOWN && currentIpBlock == 3) ipBlock4--;

		if (kDown & KEY_DRIGHT && currentIpBlock < 4) currentIpBlock++;
		if (kDown & KEY_DLEFT && currentIpBlock >= 1) currentIpBlock--;

        if (kDown & KEY_PLUS) break;

		consoleUpdate(NULL);
    }
    
    char computersIp[16];
    sprintf(computersIp, "%d.%d.%d.%d",ipBlock1, ipBlock2, ipBlock3, ipBlock4);

    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);

    if ((s=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Socket creation failed: %d\n", s);
    }

    broadcast(s, computersIp, "8192");

    printf("Connected to: %s\n", ipAddress);

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(ipAddress , &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
    }

    while (appletMainLoop()) {
        hidScanInput();
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        hidJoystickRead(&joystickLeft, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
        hidJoystickRead(&joystickRight, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);

        //DPAD Up
        if (kHeld & KEY_DUP) sendto(s, "\x1\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x1\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //DPAD Down
        if (kHeld & KEY_DDOWN) sendto(s, "\x2\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x2\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //DPAD Left
        if (kHeld & KEY_DLEFT) sendto(s, "\x3\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x3\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //DPAD Right
        if (kHeld & KEY_DRIGHT) sendto(s, "\x4\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
            else sendto(s, "\x4\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        //Minus button
        if (kHeld & KEY_MINUS) sendto(s, "\x5\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x5\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //Plus button
        if (kHeld & KEY_PLUS) sendto(s, "\x6\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x6\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //Left stick click
        if (kHeld & KEY_LSTICK) sendto(s, "\x7\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x7\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        
        //Right stick click
        if (kHeld & KEY_RSTICK) sendto(s, "\x8\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x8\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //L Shoulder
        if (kHeld & KEY_L) sendto(s, "\x9\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x9\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //R Shoulder
        if (kHeld & KEY_R) sendto(s, "\xA\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xA\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);
        
        //Touchscreen touch (XBOX Button)
        if (kHeld & KEY_TOUCH) sendto(s, "\xB\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xB\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //A button
        if (kHeld & KEY_A) sendto(s, "\xC\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xC\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //B button
        if (kHeld & KEY_B) sendto(s, "\xD\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xD\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //X button
        if (kHeld & KEY_X) sendto(s, "\xE\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xE\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //Y Button
        if (kHeld & KEY_Y) sendto(s, "\xF\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\xF\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //ZL Trigger
        if (kHeld & KEY_ZL) sendto(s, "\x10\x1", 2 , 0 , (struct sockaddr *) &si_other, slen);
        else sendto(s, "\x10\x0", 2 , 0 , (struct sockaddr *) &si_other, slen);

        //ZR Trigger
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
