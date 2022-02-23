/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <algorithm>

#include "linkapp.h"
#include "menu.h"
#include "selector.h"
#include "messagebox.h"
#include "debug.h"

using std::ifstream;
using std::ofstream;

extern char** environ;

LinkApp::LinkApp(GMenu2X *gmenu2x, const char* file):
Link(gmenu2x, MakeDelegate(this, &LinkApp::run)), file(file) {
	setCPU(gmenu2x->platform->cpu_link);
	setGamma(0);

	if (((float)(gmenu2x->platform->w)/gmenu2x->platform->h) != (4.0f/3.0f)) _scalemode = 3; // 4:3 by default
	scalemode = _scalemode;

	string line;
	ifstream f(file, std::ios_base::in);
	while (getline(f, line, '\n')) {
		line = trim(line);
		if (line == "") continue;
		if (line[0] == '#') continue;

		string::size_type position = line.find("=");
		string name = trim(line.substr(0, position));
		string value = trim(line.substr(position + 1));

		if (name == "exec") setExec(value);
		else if (name == "title") setTitle(value);
		else if (name == "description") setDescription(value);
		else if (name == "icon") setIcon(value);
		else if (name == "opk[icon]") icon_opk = value;
		else if (name == "params") setParams(value);
		else if (name == "home") setHomeDir(value);
		else if (name == "manual") setManual(value);
		else if (name == "clock") setCPU(atoi(value.c_str()));
		else if (name == "gamma") setGamma(atoi(value.c_str()));
		else if (name == "selectordir") setSelectorDir(value);
		else if (name == "selectorbrowser" && value == "false") setSelectorBrowser(false);
		else if (name == "scalemode") setScaleMode(atoi(value.c_str()));
		else if (name == "selectorfilter") setSelectorFilter(value);
		else if (name == "selectorscreens") setSelectorScreens(value);
		else if (name == "selectoraliases") setAliasFile(value);
		else if (name == "selectorelement") setSelectorElement(atoi(value.c_str()));
		else if ((name == "consoleapp") || (name == "terminal")) setTerminal(value == "true");
		else if (name == "backdrop") setBackdrop(value);
		else if (name == "autorun") autorun = (value == "true");
		else if (name == "favourite") addFavourite(value);
		// else WARNING("Unrecognized option: '%s'", name.c_str());
	}
	f.close();

	if (file_ext(exec, true) == ".opk") {
		package_type = PKG_OPK;
	} else if (file_ext(exec, true) == ".so") {
		package_type = PKG_RETROARCH;
	}

	if (autorun) {
		run();
		return;
	}

	if (iconPath.empty()) iconPath = searchIcon();
	if (manualPath.empty()) manualPath = searchManual();
	if (backdropPath.empty()) backdropPath = searchBackdrop();
}

const string LinkApp::searchManual() {
	if (!manualPath.empty()) return manualPath;
	string filename = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) filename = exec.substr(0, pos);
	filename += ".man.txt";

	string dname = dir_name(exec) + "/";

	string dirtitle = dname + base_name(dir_name(exec)) + ".man.txt";
	string linktitle = base_name(file, true);
	linktitle = dname + linktitle + ".man.txt";

	if (file_exists(linktitle)) return linktitle;
	if (file_exists(filename)) return filename;
	if (file_exists(dirtitle)) return dirtitle;

	return "";
}

const string LinkApp::searchBackdrop() {
	if (!backdropPath.empty() || !gmenu2x->confInt["skinBackdrops"]) return backdropPath;
	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec));
	string linktitle = base_name(file);
	string sublinktitle = base_name(file);

	pos = linktitle.find(".");
	if (pos != string::npos) sublinktitle = linktitle.substr(0, pos);

	pos = linktitle.rfind(".");
	if (pos != string::npos) linktitle = linktitle.substr(0, pos);

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + sublinktitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + sublinktitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + linktitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + exectitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".png");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = gmenu2x->sc.getSkinFilePath("backdrops/" + dirtitle + ".jpg");
	if (!backdropPath.empty()) return backdropPath;

	backdropPath = dir_name(exec) + "/backdrop.png";
	if (file_exists(backdropPath)) return backdropPath;

	return "";
}

const string LinkApp::searchIcon() {
	string iconpath = gmenu2x->sc.getSkinFilePath(icon, false);
	if (!iconpath.empty()) return iconpath;

	string execicon = exec;
	string::size_type pos = exec.rfind(".");
	if (pos != string::npos) execicon = exec.substr(0, pos);
	string exectitle = base_name(execicon);
	string dirtitle = base_name(dir_name(exec));
	string linktitle = base_name(file);

	vector<string> linkparts;
	split(linkparts, linktitle, ".");

	if (linkparts.size() > 2) {
		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[0] + "." + linkparts[1] + ".png", false);
		if (!iconpath.empty()) return iconpath;

		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[1] + "." + linkparts[0] + ".png", false);
		if (!iconpath.empty()) return iconpath;
	}

	if (linkparts.size() > 1) {
		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[1] + ".png", false);
		if (!iconpath.empty()) return iconpath;

		iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linkparts[0] + ".png", false);
		if (!iconpath.empty()) return iconpath;
	}

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + linktitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + exectitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = gmenu2x->sc.getSkinFilePath("icons/" + dirtitle + ".png", false);
	if (!iconpath.empty()) return iconpath;

	iconpath = dir_name(exec) + "/" + exectitle + ".png";
	if (file_exists(iconpath)) return iconpath;

	iconpath = execicon + ".png";
	if (file_exists(iconpath)) return iconpath;

	if (!gmenu2x->platform->opk.empty() || package_type == PKG_OPK) {
		return exec + "#" + icon_opk;
	} else

	return gmenu2x->sc.getSkinFilePath("icons/generic.png");
}

void LinkApp::setCPU(int mhz) {
	clock = mhz;
	if (clock != 0) clock = constrain(clock, gmenu2x->platform->cpu_min, gmenu2x->platform->cpu_max);
	edited = true;
}

void LinkApp::setGamma(int gamma) {
	gamma = constrain(gamma, 0, 100);
	edited = true;
}

bool LinkApp::targetExists() {
#if defined(TARGET_LINUX)
	return true; //For displaying elements during testing on pc
#endif
	string target = exec;
	if (!exec.empty() && exec[0] != '/' && !homedir.empty())
		target = homedir + "/" + exec;

	return file_exists(target);
}

bool LinkApp::save() {
	if (!edited) return false;

	ofstream f(file);
	if (!f.is_open()) {
		ERROR("Error while opening the file '%s' for write.", file.c_str());
		return false;
	}

	int pos = icon.find('#'); // search for "opkfile.opk#icon.png"
	if (pos != string::npos) {
		icon_opk = icon.substr(pos + 1);
	}

	if (title != "")			f << "title="			<< title			<< std::endl;
	if (description != "")		f << "description="		<< description		<< std::endl;
	if (icon != "")				f << "icon="			<< icon				<< std::endl;
	if (icon_opk != "")			f << "opk[icon]="		<< icon_opk			<< std::endl;
	if (exec != "")				f << "exec="			<< exec				<< std::endl;
	if (params != "")			f << "params="			<< params			<< std::endl;
	if (homedir != "")			f << "home="			<< homedir			<< std::endl;
	if (manual != "")			f << "manual="			<< manual			<< std::endl;
	if (clock != 0 && clock != gmenu2x->platform->cpu_link) f << "clock="	<< clock	<< std::endl;
	if (gmenu2x->platform->gamma && gamma != 0)	f << "gamma="	<< gamma			<< std::endl;
	if (selectordir != "")		f << "selectordir="		<< selectordir		<< std::endl;
	if (!selectorbrowser)		f << "selectorbrowser=false"				<< std::endl; // selectorbrowser = true by default
	if (scalemode != _scalemode)	f << "scalemode="	<< scalemode		<< std::endl; // scalemode = 0 by default
	if (selectorfilter != "")	f << "selectorfilter="	<< selectorfilter	<< std::endl;
	if (selectorscreens != "")	f << "selectorscreens="	<< selectorscreens	<< std::endl;
	if (selectorelement > 0)	f << "selectorelement="	<< selectorelement	<< std::endl;
	if (aliasfile != "")		f << "selectoraliases="	<< aliasfile		<< std::endl;
	if (backdrop != "")			f << "backdrop="		<< backdrop			<< std::endl;
	if (terminal)				f << "terminal=true"						<< std::endl;

	for (int i = 0; i < favourites.size(); i++) {
		f << "favourite=" << favourites.at(i) << std::endl;
	}

	f.close();
	return true;
}

void LinkApp::run() {
	uint32_t start = SDL_GetTicks();
	gmenu2x->confInt["selectorElement"] = -1;
	gmenu2x->confStr["selectorDir"] = "";

	while (gmenu2x->input->isActive(CONFIRM)) {
		gmenu2x->input->update();
		if (SDL_GetTicks() - start > 1400) {
			// hold press -> inverted
			if (selectordir != "")
				return launch();
			return selector();
		}
	}

	// quick press -> normal
	if (selectordir != "")
		return selector();
	return launch();
}

void LinkApp::selector(int startSelection, string selectorDir) {
	//Run selector interface
	Selector bd(gmenu2x, this->getTitle(), this->getDescription(), this->getIconPath(), this);
	bd.showDirectories = this->getSelectorBrowser();

	if (selectorDir.empty()) selectorDir = this->getSelectorDir();

	bd.setFilter(this->getSelectorFilter());

	if (startSelection > 0) bd.selected = startSelection;
	else bd.selected = this->getSelectorElement();

	if (bd.exec(selectorDir)) {
		gmenu2x->confInt["selectorElement"] = bd.selected;
		gmenu2x->confStr["selectorDir"] = bd.getDir();
		gmenu2x->confInt["lastSection"] = gmenu2x->menu->getSectionIndex();
		gmenu2x->confInt["lastLink"] = gmenu2x->menu->getLinkIndex();

		gmenu2x->writeTmp();

		string s = "";
		s += this->getSelectorDir().back();
		if (s != "/") {
			setSelectorDir(bd.getDir());
			setSelectorElement(bd.selected);
			save();
		}

		params = trim(params + " " + bd.getParams(bd.selected));

		launch(bd.getFile(bd.selected), bd.getDir());
	}
}

void LinkApp::launch(const string &selectedFile, string dir) {
	MessageBox mb(gmenu2x, gmenu2x->tr["Launching"] + " " + this->getTitle(), this->getIconPath());
	mb.setAutoHide(1);
	mb.exec();

	string command = cmdclean(exec);

	if (selectedFile.empty()) {
		gmenu2x->writeTmp();
	} else {
		if (dir.empty()) {
			dir = getSelectorDir();
		}

		if (params.empty()) {
			params = cmdclean(dir + "/" + selectedFile);
		} else {
			string origParams = params;
			params = strreplace(params, "[selFullPath]", cmdclean(dir + "/" + selectedFile));
			params = strreplace(params, "\%f", cmdclean(dir + "/" + selectedFile));
			params = strreplace(params, "[selPath]", cmdclean(dir));
			params = strreplace(params, "[selFile]", cmdclean(selectedFile));
			params = strreplace(params, "[selFileNoExt]", cmdclean(base_name(selectedFile, true)));
			params = strreplace(params, "[selExt]", cmdclean(file_ext(selectedFile, false)));
			if (params == origParams) params += " " + cmdclean(dir + "/" + selectedFile);
		}
	}

	INFO("Executing '%s' (%s %s)", title.c_str(), exec.c_str(), params.c_str());

	if (package_type == PKG_OPK) {
		string opk_mount = "umount -fl /mnt &> /dev/null; mount -o loop " + command + " /mnt";
		system(opk_mount.c_str());
		chdir("/mnt"); // Set correct working directory

		command = "/mnt/" + params;
		params = "";
	}

	else if (package_type == PKG_RETROARCH) {
		command = "retroarch -L \"" + cmdclean(command) + "\"";
	}
	else
	{
		chdir(dir_name(exec).c_str()); // Set correct working directory
	}

	// Check to see if permissions are desirable
	struct stat fstat;
	if (!stat(command.c_str(), &fstat)) {
		struct stat newstat = fstat;
		if (S_IRUSR != (fstat.st_mode & S_IRUSR)) newstat.st_mode |= S_IRUSR;
		if (S_IXUSR != (fstat.st_mode & S_IXUSR)) newstat.st_mode |= S_IXUSR;
		if (fstat.st_mode != newstat.st_mode) chmod(command.c_str(), newstat.st_mode);
	} // else, well.. we are no worse off :)

	if (params != "") command += " " + params;

	if (getCPU() != gmenu2x->platform->cpu_menu) gmenu2x->platform->setCPU(getCPU());

	if (getGamma() != 0 && getGamma() != gmenu2x->confInt["gamma"]) {
		gmenu2x->platform->setGamma(getGamma());
	}

	gmenu2x->platform->setScaleMode(scalemode);

	command = gmenu2x->platform->hwPreLinkLaunch() + command;

	if (gmenu2x->confInt["outputLogs"]) {
		params = "echo " + cmdclean(command) + " > " + cmdclean(home_path("log.txt"));
		system(params.c_str());
		command += " 2>&1 | tee -a " + cmdclean(home_path("log.txt"));
	}

	// params = this->getHomeDir();
	params = gmenu2x->confStr["homePath"];

	if (!params.empty() && dir_exists(params)) {
		command = "HOME=" + params + " " + command;
	}

	if (getTerminal()) gmenu2x->platform->enableTerminal();

	gmenu2x->quit(false);

	// execle("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL, environ);
	execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);

	//if execution continues then something went wrong and as we already called SDL_Quit we cannot continue
	//try relaunching gmenu2x
	chdir(exe_path().c_str());
	execlp("./gmenu2x", "./gmenu2x", NULL);
}

void LinkApp::setExec(const string &exec) {
	this->exec = exec;
	edited = true;
}

void LinkApp::setParams(const string &params) {
	this->params = params;
	edited = true;
}

void LinkApp::setHomeDir(const string &homedir) {
	this->homedir = homedir;
	edited = true;
}

void LinkApp::setManual(const string &manual) {
	this->manual = manualPath = manual;
	edited = true;
}

void LinkApp::setSelectorDir(const string &selectordir) {
	edited = this->selectordir != selectordir;
	this->selectordir = selectordir;
	// if (this->selectordir != "") this->selectordir = real_path(this->selectordir);
}

void LinkApp::setSelectorBrowser(bool value) {
	selectorbrowser = value;
	edited = true;
}

void LinkApp::setTerminal(bool value) {
	terminal = value;
	edited = true;
}

void LinkApp::setScaleMode(int value) {
	scalemode = value;
	edited = true;
}

void LinkApp::setSelectorFilter(const string &selectorfilter) {
	this->selectorfilter = selectorfilter;
	edited = true;
}

void LinkApp::setSelectorScreens(const string &selectorscreens) {
	this->selectorscreens = selectorscreens;
	edited = true;
}

void LinkApp::setSelectorElement(int i) {
	edited = selectorelement != i;
	selectorelement = i;
}

void LinkApp::setAliasFile(const string &aliasfile) {
	this->aliasfile = aliasfile;
	edited = true;
}

void LinkApp::renameFile(const string &name) {
	file = name;
}

void LinkApp::addFavourite(const string &fav) {
	edited = true;
	favourites.push_back(fav);
}
void LinkApp::delFavourite(const string &fav) {
	edited = true;
	favourites.erase(std::remove(favourites.begin(), favourites.end(), fav), favourites.end());
}
