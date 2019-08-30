Win::Win() : Window() {
	init();	// initialize static variables
	reflowStatic();	// initialize static layout
	reflow();	// update initial layouts
}

void Win::openFile(const lstring &files) {
	setStatusText({
		"Opened ",
		files.size()>1?"multiple files":files[0]
	});
	setFocused();
	prog->open(files);
//	feFile.setPath(fn);
}
void Win::init() {
	/** init settings **/
	setWindowGeometry({32,32,640,480});
	mFile.setText("File");
		mFileOpen.setText("Open File");
		mFileOpenDir.setText("Open Folder");
		mFileQuit.setText("Quit");
	mTools.setText("Tools");
		mToolsPrefs.setText("Preferences...");
	mHelp.setText("Help");
		mHelpAbout.setText("About...");
	out.setText("");
	lMain.setMargin(5);
	lvFiles.setHeaderText({"Status","File"});
	lvFiles.setHeaderVisible();
/*	lTab1.setMargin(5);
		lTab1a.setMargin(5);
	lTab2.setMargin(5);
	lTab3.setMargin(5);
	lblText.setText("Hello world!");
	lblT2.setText("Hello tabs!");
	lblT3.setText("Hello phoenix!");
	bImg.setText(""); clicks = 0;
	bImgText.setText("Icon+Text");
	bText.setText("Text");
*/	repaint();
/*	bar.status.setText({
		"Hello world!",
		" ",
		"What's up?"
	});
	//browser.setParent(*this).setPath("").setFilters("All files (*)");
*/
	browser.setParent(*this).setFilters({"VGM Files (*.vgm, *.vgz)","All files (*)"});
	initProc();
//	tUpdater.setEnabled();
//	tRenderer.setEnabled();
}
void Win::repaint() {}
void Win::initProc() {
	/** menu methods **/
	mFileOpen.onActivate = [&] {
		string fn = browser.setPath("").open();
		//string fn = browser.open();
		if (!fn.empty()) openFile(fn);
	};
	mFileOpenDir.onActivate = [&] {
		string fn = browser.setPath("").directory();
		//string fn = browser.open();
		if (!fn.empty()) openFile(fn);
	};
	mFileQuit.onActivate = &Application::quit;
	mToolsPrefs.onActivate = [&] {
		wOpt.setVisible();
	};
	mHelpAbout.onActivate = [&] {
		// todo: open about dialog
		MessageWindow()
		    .setTitle({"About ", prog->name})
		    .setText({
		      prog->name, " v", prog->version, "\n",
		      "Author: ", "apollolux", "\n",
		      "License: ", "ISC"
		    })
    		.information();
	};
	/** widget methods **/
/*	tUpdater.onActivate = [&](){
		mesh.rotation.x += 0.01;
		mesh.rotation.y += 0.01;
	};
	tUpdater.setInterval(1);
	tRenderer.onActivate = [&]() {
		repaint();
	};
	tRenderer.setInterval(16);
	bImg.onActivate = [&]() {
		lblT2.setText({"Clicks: ", ++clicks});
		string cmd = {"echo \"hello clicks (",clicks,")\""};
		_wsystem(utf16_t(cmd));
	};
	cnSmp.onMousePress = [&](Mouse::Button b) {
		Position* p = &cnSmp.current;
		if (b==Mouse::Button::Left) {
			uint32_t colT = 0x20ffff20;
			uint32_t before = cnSmp.at(p->x, p->y);
			cnSmp.circle(p->x, p->y, 5, colT);
			setStatusText({
				"PLOT [",
					p->x,",",
					p->y,":",
					hex<8,'0'>(before),"->",hex<8,'0'>(cnSmp.at(p->x, p->y)),
				"]"
			});
			//cnSmp.setData();
		}
		else setStatusText({
			"Button:",(unsigned)b,
			" P[",cnSmp.current.x,",",cnSmp.current.y,"]"
		});
	};
*/	/** window methods **/
	onClose = &Application::quit;
	onSize = {&Win::resize, this};
	/**** handle "drag & drop onto window" events ****/
	setDroppable();
	onDrop = [&](lstring fns) {
		if (fns.size()) {
			openFile(fns);
			/*if (directory::exists(fns[0])) {
				openFile(fns[0]);
				setStatusText({
					"Opened directory ",
					fns[0]
				});
				setFocused();
			}
			else if (file::exists(fns[0])) {
				openFile(fns[0]);
				setStatusText({
					"Opened ",
					fns[0]
				});
				setFocused();
			}*/
		}
		else {
			MessageWindow()
				.setTitle({"Drop failed - ", prog->name})
				.setText({
					"Couldn't open!"
				})
    			.information();
		}
	};
	setMenuVisible();
	//setStatusVisible();
}

void Win::reflowStatic() {
	append(mFile);
		mFile.append(mFileOpen);
		mFile.append(mFileOpenDir);
		mFile.append(mFileQuit);
	append(mTools);
		mTools.append(mToolsPrefs);
	append(mHelp);
		mHelp.append(mHelpAbout);
/*	lTabbed.append("First");
		lTabbed.setLayout(0, lTab3);
		fsT1.setText({"Framed"});
		fsT1.setLayout(lTab1a);
	lTabbed.append("Second");
		lTabbed.setLayout(1, lTab2);
	lTabbed.append("Third");
		lTabbed.setLayout(2, lTab1);
	if (sprTabs.load("res/ui-icons_2e83ff_256x240.png")) {
		lTabbed.setImage(0, sprTabs.slice(16,144,16,16));
		lTabbed.setImage(1, sprTabs.slice(144,128,16,16));
		lTabbed.setImage(2, sprTabs.slice(160,112,16,16));
		bImg.setImage(sprTabs.slice(240,96,16,16) );
		bImgText.setImage(sprTabs.slice(176,112,16,16) );
		//lvSw.setImage(5,0, spr.slice(144,96,16,16) );
	}
	else {
		bImg.setText("Icon (no icon)");
		bImgText.setText("Image+Text (no icon)");
		lblT2.setText("Hello tabs (no icons)!");
	}
*/}
void Win::reflow() {
	append(lMain);
	lMain.append(lLog, {~0,~0}, 5);
	lLog.append(lvFiles, {~0, ~0}, 5);
	lLog.append(out, {~0, ~0});
/*	lMain.remove(lTabbed); lMain.remove(feFile);
	lMain.append(lTabbed, {~0, ~0}, 5);
		lTab1.remove(fsT1); lTab1.append(fsT1, {~0, ~0});
			lTab1a.remove(lblText); lTab1a.append(lblText, {~0, 0});
		lTab2.remove(lblT2);
			lTab2.append(lblT2, {~0, 0}, 5);
			lTab2.append(lT2Btns, {~0, 0});
				lT2Btns.append(bImg, {0, 0}, 5);
				lT2Btns.append(bText, {0, ~0}, 5);
				lT2Btns.append(bImgText, {0, 0});
		lTab3.remove(lblT3);
			lTab3.append(lblT3, {~0, 0}, 5);
			lTab3.append(cnSmp, {~0, ~0});
	lMain.append(feFile, {~0, 0}); feFile.reflow();
*/	lMain.append(bar, {~0, 0});
}
void Win::resize() {
	/****
	add some resize handling here
	maybe make an interface 3 columns instead of 2 if wide enough,
	or set a tab bar to align left instead of top,
	or set buttons to only use images when small and image+text when wide
	****/
}


/**** options window ****/
void WinOptions::initProc() {
	unsigned i;
	auto &c = (prog->cfg);
	for (i=0; i<N::OPN::Types::Total; ++i) {
		opnToggles[i].setEnabled(!c.disabledOPN[i]);
		opnToggles[i].setChecked(c.dumpOPN[i]);
	}
	opnToggles[N::OPN::Types::VGI].onToggle = [&]() {c.dumpOPN[N::OPN::Types::VGI] = opnToggles[N::OPN::Types::VGI].checked();};
	opnToggles[N::OPN::Types::TFI].onToggle = [&]() {c.dumpOPN[N::OPN::Types::TFI] = opnToggles[N::OPN::Types::TFI].checked();};
	opnToggles[N::OPN::Types::TYI].onToggle = [&]() {c.dumpOPN[N::OPN::Types::TYI] = opnToggles[N::OPN::Types::TYI].checked();};
	opnToggles[N::OPN::Types::EIF].onToggle = [&]() {c.dumpOPN[N::OPN::Types::EIF] = opnToggles[N::OPN::Types::EIF].checked();};
	opnToggles[N::OPN::Types::DMP].onToggle = [&]() {c.dumpOPN[N::OPN::Types::DMP] = opnToggles[N::OPN::Types::DMP].checked();};
	opnToggles[N::OPN::Types::Y12].onToggle = [&]() {c.dumpOPN[N::OPN::Types::Y12] = opnToggles[N::OPN::Types::Y12].checked();};
	opnToggles[N::OPN::Types::GYB].onToggle = [&]() {c.dumpOPN[N::OPN::Types::GYB] = opnToggles[N::OPN::Types::GYB].checked();};
	for (i=0; i<N::OPM::Types::Total; ++i) {
		opmToggles[i].setEnabled(!c.disabledOPM[i]);
		opmToggles[i].setChecked(c.dumpOPM[i]);
	}
	opmToggles[N::OPM::Types::OPM].onToggle = [&]() {c.dumpOPM[N::OPM::Types::OPM] = opmToggles[N::OPM::Types::OPM].checked();};
}
