#include "Decryption.h"
#include "XButton.h"
#include "Button.h"
#include "ScrollPanel.h"
#include "ScrollBar.h"
#include "RadioButton.h"
#include "TextField.h"
#include "InfoText.h"

//TODO doesn't minimize right
//TODO I was able to get out of bounds while forcing with the three buttons (just go backwards over moving line)
void Decryption::animate()
{
	if (required_text_field != NULL)
	{
		if (required_text_field->required_text_pos >= required_text_field->required_text.size())
		{
			parent->force_to(75, 925);
			parent->end_of_forcing_function = []() {};
			required_text_field = NULL;
		}
	}
	
	if (parent->sticking_cursor)
	{
		if (stick_counter == 0)
		{
			stick_frames++;
			if (stick_frames > 60 * 3)
			{
				parent->sticking_cursor = false;
				parent->force_to(parent->sticking_x - 200, parent->sticking_y + 200);
				parent->end_of_forcing_function = [this]() {parent->forcing_cursor = false; parent->sticking_cursor = true; parent->sticking_x -= 200; parent->sticking_y += 200;};
				stick_frames = 0;
				stick_counter++;
			}
		}

		if (stick_counter == 1)
		{
			stick_frames++;
			if (stick_frames > 60 * 3)
			{
				parent->sticking_cursor = false;
				auto help_message = make_unique<InfoText>(580, 120, 580, 120, "white.png", "Need help using the app?", DECRYPTION_APP);
				auto help_button = make_unique<Button>(900, 120, 950, 150, "ok.png", DECRYPTION_APP, [this]() 
				{
					remove_children();
					auto button_1 = make_unique<Button>(700, 600, 750, 630, "ok.png", DECRYPTION_APP, [this]() 
					{
						remove_children();
						auto text_field = make_unique<TextField>(600, 300, 900, 400, "textfield.png", parent, DECRYPTION_APP);
						text_field->required_text = "So yes this is the message I'm going to type and I hope it comes out very nice and pretty thank you. :)";
						this->required_text_field = text_field.get();
						children.push_back(text_field.get());
						parent->to_be_added.push_back(std::move(text_field));
						parent->force_to(610, 390);
						parent->end_of_forcing_function = []() {};
					}, []() {});
					auto button_2 = make_unique<Button>(700, 400, 750, 430, "ok.png", DECRYPTION_APP, []() {}, []() {});
					auto button_3 = make_unique<Button>(700, 200, 750, 230, "ok.png", DECRYPTION_APP, []() {}, []() {});
					children.push_back(button_1.get());
					children.push_back(button_2.get());
					children.push_back(button_3.get());
					parent->to_be_added.push_back(std::move(button_1));
					parent->to_be_added.push_back(std::move(button_2));
					parent->to_be_added.push_back(std::move(button_3));
					parent->force_to(1500, 900);
					parent->end_of_forcing_function = [this]() {parent->force_to(720, 610); /*parent->end_of_forcing_function = []() {};*/};
				}, []() {});
				children.push_back(help_message.get());
				children.push_back(help_button.get());
				parent->to_be_added.push_back(std::move(help_message));
				parent->to_be_added.push_back(std::move(help_button));
				parent->process_control_changes();
				parent->force_to(920, 130);
				parent->end_of_forcing_function = []() {};
				stick_frames = 0;
				stick_counter++;
			}
		}
	}

	frames_since_open++;
	if (screen_number == 0 && frames_since_open >= 180)
	{
		screen_number++;
		auto minimize = parent->minimize_function(DECRYPTION_APP, "icon_decrypt.png");
		parent->to_be_added.push_back(make_unique<ScreenElement>(560.0, 744.0, 1360.0, 780.0, "silver.png", DECRYPTION_APP));
		parent->to_be_added.push_back(make_unique<XButton>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", parent, DECRYPTION_APP));
		parent->to_be_added.push_back(make_unique<Button>(1286.0, 747.0, 1317.0, 778.0, "minimize.png", DECRYPTION_APP, minimize, [](){}));
		parent->to_be_added.push_back(make_unique<ScreenElement>(570.0, 104.0, 1350.0, 744.0, "silver.png", DECRYPTION_APP));
		auto button = make_unique<Button>(750.0, 500.0, 950.0, 530.0, "ok.png", DECRYPTION_APP,
			[this]()
		{
			screen_number++;
			remove_children();
			auto scroll_panel = make_unique<ScrollPanel>(590.0, 200.0, 1000.0, 700.0, parent, DECRYPTION_APP, true);
			auto scroll_bar = make_unique<ScrollBar>(1000.0, 300.0, 1020.0, 680.0, "scrollbar.png", DECRYPTION_APP, scroll_panel.get());
			auto radio_button = make_unique<RadioButton>(590.0, 120.0, 620.0, 150.0, "green.png", parent, DECRYPTION_APP);
			auto legal_button = make_unique<Button>(1000.0, 620.0, 1050.0, 650.0, "ok.png", DECRYPTION_APP, [this]()
				{
					screen_number++;
					//parent->start_forcing_cursor(-1, 800, -1, 805, 1, -500, 1, 800, true);
					//parent->start_forcing_cursor(-1, 800, -1, 805, 1, 800, 1, -500, false);
					//parent->force_to(parent->prev_mouse_x + 2000, parent->prev_mouse_y - 2000);
					//parent->force_to(1700, 2);
					remove_children();
					auto file_name_field = make_unique<TextField>(600, 700, 800, 750, "textfield.png", parent, DECRYPTION_APP, []() {}, [this](){parent->looking_for_stick = true;});
					auto decrypt_button = make_unique<Button>(850, 600, 950, 650, "ok.png", DECRYPTION_APP, [this]() {parent->start_application(INFO_BOX, "Please enter a file name");}, []() {});
					children.push_back(file_name_field.get());
					children.push_back(decrypt_button.get());
					parent->to_be_added.push_back(std::move(file_name_field));
					parent->to_be_added.push_back(std::move(decrypt_button));
				}, []() {});
			legal_button->grey_texture = "red.png";
			legal_button->greyed_out = true;
			legal_button_ptr = legal_button.get();
			legal_radio_ptr = radio_button.get();
			scroll_panel->scroll_bar = scroll_bar.get();
			scroll_panel->change_website("useful");
			children.push_back(scroll_panel.get());
			children.push_back(scroll_bar.get());
			children.push_back(legal_radio_ptr);
			children.push_back(legal_button_ptr);
			parent->to_be_added.push_back(std::move(scroll_panel));
			parent->to_be_added.push_back(std::move(scroll_bar));
			parent->to_be_added.push_back(std::move(radio_button));
			parent->to_be_added.push_back(std::move(legal_button));
		}, []() {});
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
		parent->process_control_changes();
	}

	if (screen_number == 2)
	{
		if (legal_radio_ptr->has_selection())
			legal_button_ptr->greyed_out = false;
	}
}

void Decryption::draw(TextureManager* texture_manager)
{
	if (screen_number == 0)
	{
		ScreenElement splash_image(x1, y1, x2, y2, "e1.png");
		splash_image.draw(texture_manager);
	}

	if (screen_number == 1)
	{

	}
}

void Decryption::remove_children()
{
	for (int i = 0; i < parent->screen_elements.size(); ++i)
	{
		for (int j = 0; j < children.size(); ++j)
		{
			if (children[j] == parent->screen_elements[i].get())
				parent->screen_elements[i]->marked_for_deletion = true;
		}
	}

	children.clear();
}