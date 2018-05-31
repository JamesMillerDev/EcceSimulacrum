#ifndef LOADING_INDICATOR
#define LOADING_INDICATOR

#include "ObjModel.h"
#include "Computer.h"
#include "ScrollPanel.h"
#include "fonts.h"

struct LoadingIndicator : public ScreenElement
{
	ScrollPanel* scroll_panel;
	LoadingIndicator(ScrollPanel* _scroll_panel) : ScreenElement(0, 0, 0, 0, "", BROWSER), scroll_panel(_scroll_panel) {}
	void animate()
	{
		if (!scroll_panel->currently_loading_site)
			marked_for_deletion = true;
	}

	void draw(TextureManager* texture_manager)
	{
		if (!scroll_panel->currently_loading_site)
			return;

		auto strings = split(scroll_panel->site_to_be_loaded, '/');
		string connecting_to = split(scroll_panel->website_names[strings[0]], '/')[0];
		if (connecting_to == "q")
			connecting_to = strings[0];

		if (scroll_panel->trying_to_load != "")
			connecting_to = split(scroll_panel->trying_to_load, '/')[0];

		string message = "Connecting to " + connecting_to + "...";
		vector<float> widths = draw_string(texture_manager, 32, message, 0, 0, false);
		float width = widths[widths.size() - 1];
		width += 6;
		ScreenElement beige = ScreenElement(0.0, 56.0, width + 30, 73.0 + 10, "loadinggradient.png");
		beige.draw(texture_manager);
		ScreenElement black1 = ScreenElement(0, 73 + 10, width + 30, 74 + 10, "black.png");
		ScreenElement black2 = ScreenElement(width + 30, 56, width + 30 + 1, 74 + 10, "black.png");
		black1.draw(texture_manager);
		black2.draw(texture_manager);
		draw_string(texture_manager, 32, message, 3.0, 60.0);
		int num = (scroll_panel->site_change_frame % 36 + 1) / 2;
		if (num == 0)
			num = 18;

		ScreenElement loading = ScreenElement(width, 58, -1, -1, "sitechange" + to_string(num) + ".png");
		loading.draw(texture_manager);
	}
};

#endif