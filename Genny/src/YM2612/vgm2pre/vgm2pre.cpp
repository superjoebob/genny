#include "vgm2pre.hpp"
#include "app.cpp"
#include "sub.ym2612.cpp"

/** open a file **/
void Prog::open(const lstring &files) {
	close();
	unsigned s = files.size();
	if (!cfg.headless) {
		win->bar.setProgress(0);
		win->bar.bDo.setEnabled(false);
	}
	if (s>0) {
		proc.init();
		unsigned i = 0; string msg;
		for (auto &fn : files) {
			msg = string{"Processed \"",fn,"\""};
			if (directory::exists(fn)) {
				msg = string{"Processed \"",fn,"\" as directory"};
				proc.processDirectory(string{fn,"/"});
				/*MessageWindow()
					.setTitle(name)
					.setText({
						fn," is a directory!"
					})
					.warning();*/
			}
			else {
				string ext = extension(fn);
				if (ext=="vgm"||ext=="vgz") {
					proc.processFile(fn);
					//win->setTitle({notdir(fn)," - ", name});
				}
				else {
					msg = string{
						"File \"",notdir(fn),"\" does not have a supported extension!"
					};
					if (!cfg.headless) MessageWindow()
						.setTitle(name)
						.setText(msg)
						.warning();
				}
			}
			if (!cfg.headless) {
				win->bar.setProgress((unsigned)((100*++i)/s));
				win->bar.setStatus(msg);
			}
			else print(msg,"\n");
		}
		if (!cfg.headless) win->setTitle({
			"Handling ",proc.filelist.size()," file(s)",
			" - ",name
		});
		if (proc.filelist.size()>0) {
			msg = string{
				proc.filelist.size(),
				" file",(proc.filelist.size()!=1?"s":""),
				" ready for dumping"
			};
			if (!cfg.headless) {
				win->lvFiles.reset();
				string fl = "";
				for (auto &f : proc.filelist) {
					//fl.append((string){f,"\n"});
					win->lvFiles.append({"Ready",f});
				}
				win->lvFiles.autoSizeColumns();
				win->out.setText(fl);
				win->bar.setStatus(msg);
				win->bar.bDo.setEnabled(true);
				/*MessageWindow()
					.setTitle(name)
					.setText({
						"Instrument list ready for dumping!"
					})
					.information();*/
			}
			else {
				print(msg,"\n");
				if (proc.finalize()) print("Dumping complete.\n");
				else print("Dumping failed.\n");
				Application::quit();
			}
		}
		else {
			msg = "No supported files found.";
			if (!cfg.headless) win->bar.setStatus(msg);
			else {
				print(msg,"\n");
				Application::quit();
			}
		}
	}
	else if (cfg.headless) Application::quit();
}

/** close a file **/
void Prog::close() {
	if (!cfg.headless) win->setTitle(name);
}

/** open the GUI **/
void Prog::interactiveMode(const lstring &files) {
	if (!cfg.headless) {
		win = new Win;
		win->setVisible();
		win->setFocused();
	}
	close();
	if (files.size()>0) open(files);
	Application::run();
}

/** print out program usage **/
void Prog::usage() {
	print(
		"Usage: vgm2pre [opts] [<infile> [<infile> [...]]]\n",
		"\tOptions include:\n",
		"\t-pOPN ???????\tToggle OPN preset output formats\n",
		"\t-eOPN\tUse extended OPN preset comparison\n"
		//"\t",argprefix,"eOPM\tUse extended OPM preset comparison\n",
		//"Graphical version soon!\n"
	);
	print(
		name,"\n",
		"author: apollolux\n",
		"license: ISC\n",
		"\n"
	);
}

/** initialize program variables **/
void Prog::init() {
	prog = this;
	name = "VGM2Pre";
	version = 1.0;
	cfg.headless = false;
	cfg.dumpOPN[N::OPN::Types::VGI] = true;
	cfg.dumpOPN[N::OPN::Types::TFI] = true;
	cfg.dumpOPN[N::OPN::Types::TYI] = true;
	cfg.dumpOPN[N::OPN::Types::EIF] = true;
	cfg.dumpOPN[N::OPN::Types::DMP] = true;
	cfg.dumpOPN[N::OPN::Types::Y12] = true;
	cfg.dumpOPN[N::OPN::Types::GYB] = false;
	//cfg.disabledOPN[N::OPN::Types::Y12] = true;
	//cfg.disabledOPN[N::OPN::Types::GYB] = true;
	//cfg.dumpOPM[N::OPM::Types::OPM] = true;
	cfg.disabledOPM[N::OPM::Types::OPM] = true;
	Application::setName(name);
}

/** constructor **/
Prog::Prog(int argc, char **argv) {
	init();
	lstring files;
	if (argc>1) {
		unsigned i = 1; string pf, arg;
		arg = {argv[i]};
		if (argc>=3&&arg=="-pOPN") {
			pf = {argv[++i]};
			if (pf.match("???????")) {
				cfg.dumpOPN[N::OPN::Types::VGI] = pf[0]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::TFI] = pf[1]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::TYI] = pf[2]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::EIF] = pf[3]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::DMP] = pf[4]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::Y12] = pf[5]=='1'?true:false;
				cfg.dumpOPN[N::OPN::Types::GYB] = pf[6]=='1'?true:false;
				print("Setting OPN presets to ",pf,"\n");
			}
			else print("Invalid argument to pOPN: ",pf,"\n");
			++i;
			cfg.headless = true;
		}
		for (i; i<argc; ++i) {
			if (directory::exists(argv[i])) files.append(argv[i]);
			else if (file::exists(argv[i])) files.append(argv[i]);
		}
		if (!cfg.headless) {
			interactiveMode(files);
			return;
		}
	}
	else if (argc==1) {
		if (!cfg.headless) {
			interactiveMode(files);
			return;
		}
	}
	if (cfg.headless&&files.size()>0) {
		interactiveMode(files);return;//open(files);
		//Application::run();
	}
	else usage();
}


int main(int argc, char **argv) {
	#if defined(PLATFORM_WINDOWS)
	utf8_args(argc, argv);
	#endif
	new Prog(argc, argv);
	delete prog;
	Application::quit();
	return 0;
}
