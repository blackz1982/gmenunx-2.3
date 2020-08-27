#ifndef HW_RETROGAME_H
#define HW_RETROGAME_H

volatile uint32_t *memregs;
int SOUND_MIXER = SOUND_MIXER_READ_VOLUME;

uint8_t getMMCStatus() {
	if (memdev > 0 && !(memregs[0x10500 >> 2] >> 0 & 0b1)) return MMC_INSERT;
	return MMC_REMOVE;
}

uint8_t getUDCStatus() {
	if (memdev > 0 && (memregs[0x10300 >> 2] >> 7 & 0b1)) return UDC_CONNECT;
	return UDC_REMOVE;
}

uint8_t getTVOutStatus() {
	if (memdev > 0 && !(memregs[0x10300 >> 2] >> 25 & 0b1)) return TV_CONNECT;
	return TV_REMOVE;

	// if (memdev > 0 && fwType == "RETROARCADE") return !(memregs[0x10300 >> 2] >> 6 & 0b1);
	// else if (memdev > 0) return !(memregs[0x10300 >> 2] >> 25 & 0b1);
	// return false;
}

// char *entryPoint() {
// 	static char buf[10] = { 0 };
// 	FILE *f = fopen("/dev/mmcblk0", "r");
// 	fseek(f, 0x400014, SEEK_SET); // Set the new position
// 	if (f) {
// 		for (int i = 0; i < 4; i++) {
// 			int c = fgetc(f);
// 			snprintf(buf + strlen(buf), sizeof(buf), "%02X", c);
// 		}
// 	}
// 	fclose(f);
// 	// if (!strcmp(buf, "80015840")) return "JZ4760B";

// 	if (!strcmp(buf, "800155C0")) return "JZ4760B";
// 	else if (!strcmp(buf, "80015560")) return "JZ4760";
// 	else return "Unknown";
// }

uint8_t getVolumeMode(uint8_t vol) {
	if (!vol) return VOLUME_MODE_MUTE;
	else if (memdev > 0 && !(memregs[0x10300 >> 2] >> 6 & 0b1)) return VOLUME_MODE_PHONES;
	return VOLUME_MODE_NORMAL;
}

void printbin(const char *id, int n) {
	printf("%s: 0x%08x ", id, n);
	for(int i = 31; i >= 0; i--) {
		printf("%d", !!(n & 1 << i));
		if (!(i % 8)) printf(" ");
	}
	printf("\e[0K\n");
}

static uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	if (memdev > 0) {
		// printf("\e[s\e[1;0f\e[1;32m\n");
		// printf("               3          2          1          0\n");
		// printf("              10987654 32109876 54321098 76543210\n");
		// printbin("0", memregs[0x09600 >> 2]);
		// printbin("1", memregs[0x09700 >> 2]);
		// printbin("2", memregs[0x09800 >> 2]);
		// printbin("3", memregs[0x09900 >> 2]);
		// printbin("4", memregs[0x09a00 >> 2]);
		// printbin("5", memregs[0x09b00 >> 2]);
		// printbin("6", memregs[0x09c00 >> 2]);
		// printbin("7", memregs[0x09d00 >> 2]);
		// printbin("8", memregs[0x09e00 >> 2]);
		// printbin("9", memregs[0x09f00 >> 2]);
		// printbin("A", memregs[0x10000 >> 2]);
		// printbin("B", memregs[0x10100 >> 2]);
		// printbin("C", memregs[0x10200 >> 2]);
		// printbin("D", memregs[0x10300 >> 2]);
		// printbin("E", memregs[0x10400 >> 2]);
		// printbin("F", memregs[0x10500 >> 2]);

		// printf("\n\e[30;0m\e[K\e[u");

		udcStatus = getUDCStatus();
		if (udcPrev != udcStatus) {
			udcPrev = udcStatus;
			InputManager::pushEvent(udcStatus);
		}

		mmcStatus = getMMCStatus();
		if (mmcPrev != mmcStatus) {
			mmcPrev = mmcStatus;
			InputManager::pushEvent(mmcStatus);
		}

		// tvOutStatus = getTVOutStatus();
		// if (tvOutPrev != tvOutStatus) {
		// 	tvOutPrev = tvOutStatus;
		// 	InputManager::pushEvent(tvOutStatus);
		// }

		volumeMode = getVolumeMode(1);
		if (volumeModePrev != volumeMode) {
			volumeModePrev = volumeMode;
			InputManager::pushEvent(PHONES_CONNECT);
		}

		// volumeMode = getVolumeMode(confInt["globalVolume"]);
		// if (volumeModePrev != volumeMode && volumeMode == VOLUME_MODE_PHONES) {
		// 	setVolume(min(70, confInt["globalVolume"]), true);
		// }
		// volumeModePrev = volumeMode;
	}
	return interval;
}




class hwGMenu2X : public GMenu2X {
public:
	void setTVOut(unsigned int TVOut) {
		system("echo 0 > /proc/jz/tvselect; echo 0 > /proc/jz/tvout"); // always reset tv out
		if (TVOut == TV_NTSC)		system("echo 2 > /proc/jz/tvselect; echo 1 > /proc/jz/tvout");
		else if (TVOut == TV_PAL)	system("echo 1 > /proc/jz/tvselect; echo 1 > /proc/jz/tvout");
	}

	void tvOutDialog(int TVOut) {
		if (TVOut < 0){
			MessageBox mb(this, tr["TV-out connected. Enable?"], "skin:icons/tv.png");
			mb.setButton(TV_NTSC, tr["NTSC"]);
			mb.setButton(TV_PAL,  tr["PAL"]);
			mb.setButton(TV_OFF,  tr["OFF"]);
			TVOut = mb.exec();
		}

		setTVOut(TVOut);

		switch (TVOut) {
			case TV_NTSC:
			case TV_PAL:
				setBacklight(0);
				return;
			default:
				setBacklight(confInt["backlight"]);
				break;
		}
	}

	void udcDialog() {
		// if (!fileExists("/lib/modules/g_ether.ko")) return;

		// if (!fileExists("/sys/devices/platform/musb_hdrc.0/gadget/gadget-lun1/file")) {
		// 	// MessageBox mb(this, tr["This device does not support USB mount."], "skin:icons/usb.png");
		// 	// mb.setButton(CONFIRM,  tr["Charger"]);
		// 	// mb.exec();
		// 	return;
		// }
		powerManager->setPowerTimeout(0);
		int option;
		if (confStr["usbMode"] == "Network") option = MANUAL;
		else if (confStr["usbMode"] == "Storage") option = CONFIRM;
		else if (confStr["usbMode"] == "Charger") option = CANCEL;
		else {
			MessageBox mb(this, tr["USB mode"], "skin:icons/usb.png");
			mb.setButton(MANUAL, tr["Network"]);
			mb.setButton(CANCEL,  tr["Charger"]);
			mb.setButton(CONFIRM, tr["Storage"]);
			option = mb.exec();
		}

		if (option == MANUAL) { // network
			INFO("Enabling usb0 networking device");
			system("rmmod g_file_storage; modprobe g_ether; ifdown usb0; ifup usb0");
			// system("rmmod g_ether; rmmod g_file_storage; modprobe g_ether; ifdown usb0; ifup usb0");
			// system("rmmod g_file_storage; modprobe g_ether; ifup usb0");
			iconInet = sc.skinRes("imgs/inet.png");
		} else if (option == CONFIRM) { // storage
			INFO("Enabling gadget-lun storage device");
			quit();
			execlp("/bin/sh", "/bin/sh", "-c", "/etc/init.d/S80recovery storage nopoweroff", NULL);
			chdir(getExePath().c_str());
			execlp("./gmenu2x", "./gmenu2x", NULL);
			return;
		}
	}

	void hwInit() {
		setenv("SDL_NOMOUSE", "1", 1);
		memdev = open("/dev/mem", O_RDWR);
		if (memdev < 0) WARNING("Could not open /dev/mem");

		if (memdev > 0) {
			memregs = (uint32_t*)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x10000000);
			if (memregs == MAP_FAILED) {
				ERROR("Could not mmap hardware registers!");
				close(memdev);
			}
		}

	// 	struct fb_var_screeninfo vinfo;
	// 	int fbfd = open("/dev/fb0", O_RDWR);
	// 	if (fbfd >= 0 && ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) >= 0) {
	// 		resX = vinfo.xres;
	// 		resY = vinfo.yres;
	// 		// DEBUG("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
	// 	}
	// 	close(fbfd);

	// #if defined(TARGET_RETROGAME)
	// 	if (resX == 320 && resY == 480) {
	// 		resY = 240;
	// 		// FB_SCREENPITCH = 2;
	// 		FB_SCREENPITCH = 1;
	// 		fwType = "RETROGAME";
	// 	}
	// 	else if (resX == 480 && resY == 272) {
	// 		fwType = "RETROARCADE";
	// 	}
	// #elif defined(TARGET_PC)
		resX = 320;
		resY = 240;
	// #endif
	//	INFO("Resolution: %dx%d", resX, resY);

		INFO("RETROGAME Init Done!");
	}

	int32_t getBatteryStatus() {
		char buf[32] = "-1";
		FILE *f = fopen("/proc/jz/battery", "r");
		if (f) {
			fgets(buf, sizeof(buf), f);
		}
		fclose(f);
		return atol(buf);
	}

	uint16_t getBatteryLevel() {
		int32_t val = getBatteryStatus();

		if (fwType != "RETROARCADE" && confStr["batteryType"] == "BL-5B") {
			if ((val > 10000) || (val < 0)) return 6;
			else if (val > 4000) return 5; // 100%
			else if (val > 3900) return 4; // 80%
			else if (val > 3800) return 3; // 60%
			else if (val > 3730) return 2; // 40%
			else if (val > 3600) return 1; // 20%
			return 0; // 0% :(
		}

		val = constrain(val, 0, 4500);

		bool needWriteConfig = false;
		int32_t max = confInt["maxBattery"];
		int32_t min = confInt["minBattery"];

		if (val > max) {
			needWriteConfig = true;
			max = confInt["maxBattery"] = val;
		}
		if (val < min) {
			needWriteConfig = true;
			min = confInt["minBattery"] = val;
		}

		if (needWriteConfig)
			writeConfig();

		if (max == min)
			return 0;

		// return 5 - 5*(100-val)/(100);
		return 5 - 5 * (max - val) / (max - min);
	}

	void setCPU(uint32_t mhz) {
		// mhz = constrain(mhz, CPU_CLK_MIN, CPU_CLK_MAX);
		if (memdev > 0) {
			DEBUG("Setting clock to %d", mhz);

			uint32_t m = mhz / 6;
			memregs[0x10 >> 2] = (m << 24) | 0x090520;
			INFO("Set CPU clock: %d", mhz);
			SDL_Delay(50);
			setTVOut(TVOut);
		}
	}

	int getBacklight() {
		char buf[32] = "-1";
		FILE *f = fopen("/proc/jz/lcd_backlight", "r");
		if (f) {
			fgets(buf, sizeof(buf), f);
		}
		fclose(f);
		return atoi(buf);
	}
};

#endif