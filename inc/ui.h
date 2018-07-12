#ifndef UI_H
#define UI_H

#include <string>
#include <vector>

#include "data.h"
#include "ui/button.h"

namespace ui
{
	void prepMenus();
	void loadTitleMenu();
	void showMessage(const std::string& mess);
	void drawTopBar(const std::string& info);
	void runApp(const uint32_t& down, const uint32_t& held);

	//These are locked into because the archive is opened
	std::u16string getFolder(const data::titleData& dat, const uint32_t& mode, const FS_Archive& arch, const bool& newFolder);
	void advMode(const FS_Archive& arch);

	class menu
	{
		public:
			void addOpt(const std::string& add);
			void reset();
			void setSelected(const int& newSel);

			void handleInput(const uint32_t& key, const uint32_t& held);
			void draw(const int& x, const int&y, const uint32_t& baseClr, const uint32_t& rectWidth);

			int getSelected() { return selected; }
			unsigned getCount() { return opt.size(); }
			std::string getOpt(const int& g) { return opt[g]; }

		private:
			uint8_t clrSh = 0;
			bool clrAdd = true;
			int selected = 0, start = 0;
			int fc = 0;
			std::vector<std::string> opt;
	};

	class progressBar
	{
		public:
			progressBar(const uint32_t& _max);
			void update(const uint32_t& _prog);
			void draw(const std::string& text);

		private:
			float max, prog, width;
	};

	bool confirm(const std::string& mess);
}

#endif // UI_H
