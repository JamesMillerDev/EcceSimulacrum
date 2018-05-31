#include "Decryption.h"
#include "XButton.h"
#include "Button.h"
#include "ScrollPanel.h"
#include "ScrollBar.h"
#include "RadioButton.h"
#include "TextField.h"
#include "InfoText.h"
#include "keymappings.h"

//TODO I was able to get out of bounds while forcing with the three buttons (just go backwards over moving line)
//TODO during one run I was unable to click on the button that takes you to the three buttons
//TODO right now you can just backspace as much as you want during the forced message.
//TODO there's a link right now near the bottom of the scrolling page, clicking seems to do nothing
//TODO shouldn't be able to highlight during required message for text box
//TODO shouldn't be able to do anything to text box after you're done typing the message
//TODO sometimes get stuck, just replace whole forcing with sphere movement (caused by click text box on the way up, this causes looking for stick so you can't get all the way to the bottom)
//TODO put cursor right in middle of required text box so you can't click outside it, or else disable clicking altogether while typing required message
void Decryption::animate()
{
	if (required_text_field != NULL)
	{
		if (required_text_field->required_text_pos >= required_text_field->required_text.size())
		{
			parent->force_to(75, 925);
			//parent->decrypt_forcing = false;
			//parent->end_of_forcing_function = [this]() {this->parent->forcing_cursor = false;};
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
				parent->force_to(560, 740);
				parent->end_of_forcing_function = [this]() {parent->forcing_cursor = false; parent->sticking_cursor = true; parent->sticking_x = 560; parent->sticking_y = 740;};
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
				auto help_message = make_unique<ScreenElement>(600, 242, -1, -1, "qdecrypt4.png", DECRYPTION_APP);
				auto help_button = make_unique<Button>(850, 242, 850 + 118, 282, "okblocky.png", DECRYPTION_APP, [this]() 
				{
					remove_children();
					auto header = make_unique<ScreenElement>(600, 800, -1, -1, "qdecrypt5.png", DECRYPTION_APP);
					auto message_1 = make_unique<ScreenElement>(600, 720, -1, -1, "qdecrypt6.png", DECRYPTION_APP);
					auto button_1 = make_unique<Button>(1000, 700, 1118, 740, "okblocky.png", DECRYPTION_APP, [this]() 
					{
						remove_children();
						auto message = make_unique<ScreenElement>(600, 600, -1, -1, "qdecrypt9.png", DECRYPTION_APP);
						auto text_field = make_unique<TextField>(600, 500, 600 + 229, 540, "textfieldblocky.png", parent, DECRYPTION_APP);
						//text_field->required_text = "So yes this is the message I'm going to type and I hope it comes out very nice and pretty thank you. :)";
						text_field->required_text = "Yes, I would like to report a crime. I was trying to violate someone's privacy. I am so sorry. I am so ashamed of what I did. That's why I feel compelled to say this now, to clear my name, to maek it clear that this won't happen again. I\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bake it clear that this won't happen again. I'm sorry, I realized I made an error in an earlier word and I had to go back and correct it. As I was saying, I want to guarantee that this won't happen again. I will**/ sorry, that was a mistake. I will never attempt to violate someone's privacy like that again.";
						this->required_text_field = text_field.get();
						children.push_back(text_field.get());
						children.push_back(message.get());
						parent->to_be_added.push_back(std::move(text_field));
						parent->to_be_added.push_back(std::move(message));
						parent->force_to(620, 540);
						parent->end_of_forcing_function = []() {};
					}, []() {}, "", "invisible.png", "okblockypressed.png");
					auto message_2 = make_unique<ScreenElement>(600, 520, -1, -1, "qdecrypt7.png", DECRYPTION_APP);
					auto button_2 = make_unique<Button>(1000, 500, 1118, 540, "okblocky.png", DECRYPTION_APP, []() {}, []() {}, "", "invisible.png", "okblockypressed.png");
					auto message_3 = make_unique<ScreenElement>(600, 320, -1, -1, "qdecrypt8.png", DECRYPTION_APP);
					auto button_3 = make_unique<Button>(1000, 300, 1118, 340, "okblocky.png", DECRYPTION_APP, []() {}, []() {}, "", "invisible.png", "okblockypressed.png");
					children.push_back(header.get());
					children.push_back(message_1.get());
					children.push_back(message_2.get());
					children.push_back(message_3.get());
					children.push_back(button_1.get());
					children.push_back(button_2.get());
					children.push_back(button_3.get());
					parent->to_be_added.push_back(std::move(button_1));
					parent->to_be_added.push_back(std::move(button_2));
					parent->to_be_added.push_back(std::move(button_3));
					parent->to_be_added.push_back(std::move(header));
					parent->to_be_added.push_back(std::move(message_1));
					parent->to_be_added.push_back(std::move(message_2));
					parent->to_be_added.push_back(std::move(message_3));
					parent->force_to(500, 900);
					parent->end_of_forcing_function = [this]() {parent->force_to(1020, 720); parent->end_of_forcing_function = []() {};};
				}, []() {}, "", "invisible.png", "okblockypressed.png");
				children.push_back(help_message.get());
				children.push_back(help_button.get());
				parent->to_be_added.push_back(std::move(help_message));
				parent->to_be_added.push_back(std::move(help_button));
				parent->process_control_changes();
				parent->force_to(860, 250);
				parent->end_of_forcing_function = []() {};
				stick_frames = 0;
				stick_counter++;
			}
		}
	}

	frames_since_open++;
	if (screen_number == 0 && frames_since_open >= 180)
	{
		remove_children();
		parent->process_control_changes();
		screen_number++;
		auto minimize = parent->minimize_function(DECRYPTION_APP, "icon_decrypt.png");
		//parent->to_be_added.push_back(make_unique<ScreenElement>(560.0, 744.0, 1360.0, 780.0, "silver.png", DECRYPTION_APP));
		//parent->to_be_added.push_back(make_unique<XButton>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", parent, DECRYPTION_APP));
		//parent->borders_title_x(570, 104, 1350, 744, DECRYPTION_APP);
		//parent->to_be_added.push_back(make_unique<Button>(1286.0, 747.0, 1317.0, 778.0, "minimize.png", DECRYPTION_APP, minimize, [](){}));
		parent->to_be_added.push_back(make_unique<ScreenElement>(960 - 390, 540 - 320, 960 + 390, 540 + 320, "decryptbackground.png", DECRYPTION_APP));
		//auto warning_message = make_unique<InfoText>(600, 300, 900, 600, "invisible.png", "QDecrypt will close all other open applications.", DECRYPTION_APP);
		//children.push_back(warning_message.get());
		//parent->to_be_added.push_back(std::move(warning_message));
		auto warning_message = make_unique<ScreenElement>(0, 0, 714, 25, "qdecrypt1.png", DECRYPTION_APP);
		warning_message->center();
		warning_message->translate(0, 300);
		children.push_back(warning_message.get());
		parent->to_be_added.push_back(std::move(warning_message));
		auto button = make_unique<Button>(1212, 242, 1212 + 118, 242 + 40, "okblocky.png", DECRYPTION_APP,
			[this]()
		{
			screen_number++;
			remove_children();
			parent->close_for_decrypt();
			parent->whitelist.push_back("eula");
			auto black = make_unique<ScreenElement>(960 - 370 - 1, 260 + 30 - 1, 960 + 370 + 1, 800 + 30 + 1, "black.png", DECRYPTION_APP);
			children.push_back(black.get());
			parent->to_be_added.push_back(std::move(black));
			auto scroll_panel = make_unique<ScrollPanel>(960 - 370, 260 + 30, 960 + 370, 800 + 30, parent, DECRYPTION_APP, true);
			auto scroll_bar = make_unique<ScrollBar>(960 + 350, 300.0, 960 + 370, 680.0, "scrollbar.png", DECRYPTION_APP, scroll_panel.get());
			auto scroll_background = make_unique<ScreenElement>(960 + 350, 260 + 30, 960 + 370, 800 + 30, "scrollbackground.png", DECRYPTION_APP);
			auto legal_button = make_unique<Button>(1212, 242, 1212 + 118, 242 + 40, "okblocky.png", DECRYPTION_APP, [this]()
			{
				screen_number++;
				//parent->start_forcing_cursor(-1, 800, -1, 805, 1, -500, 1, 800, true);
				//parent->start_forcing_cursor(-1, 800, -1, 805, 1, 800, 1, -500, false);
				//parent->force_to(parent->prev_mouse_x + 2000, parent->prev_mouse_y - 2000);
				//parent->force_to(1700, 2);
				remove_children();
				auto file_name_field = make_unique<TextField>(600, 550, 600 + 229, 590, "textfieldblocky.png", parent, DECRYPTION_APP, []() {}, [this]() {parent->looking_for_stick = true;});
				auto message = make_unique<ScreenElement>(600, 610, -1, -1, "qdecrypt3.png", DECRYPTION_APP);
				auto decrypt_button = make_unique<Button>(1212, 242, 1212 + 118, 242 + 40, "okblocky.png", DECRYPTION_APP, [this]() {parent->start_application(INFO_BOX, "Please enter a file name");}, []() {}, "", "invisible.png", "okblockypressed.png");
				children.push_back(message.get());
				children.push_back(file_name_field.get());
				children.push_back(decrypt_button.get());
				parent->to_be_added.push_back(std::move(file_name_field));
				parent->to_be_added.push_back(std::move(decrypt_button));
				parent->to_be_added.push_back(std::move(message));
			}, []() {}, "", "invisible.png", "okblockypressed.png");
			legal_button->grey_texture = "okblockygrey.png";
			legal_button->greyed_out = true;
			legal_button_ptr = legal_button.get();
			auto check_box = make_unique<Button>(960 - 370, 252, 960 - 350, 272, "checkbox.png", DECRYPTION_APP, [this]() {this->legal_button_ptr->greyed_out = !this->legal_button_ptr->greyed_out;}, []() {}, "", "checkboxlight.png", "checkboxdark.png");
			check_box->check_box = true;
			check_box->checked = false;
			auto agree_message = make_unique<ScreenElement>(617, 242, -1, -1, "qdecrypt2.png", DECRYPTION_APP);
			children.push_back(agree_message.get());
			parent->to_be_added.push_back(std::move(agree_message));
			scroll_panel->scroll_bar = scroll_bar.get();
			auto scroll_panel_pointer = scroll_panel.get();
			scroll_bar->min_y = 260 + 50;
			scroll_bar->max_y = 800 + 10;
			scroll_panel->change_website("eula");
			auto up_arrow = make_unique<Button>(960 + 350, 800 + 3, 960 + 370, 800 + 30, "arrow.png", DECRYPTION_APP,
				[scroll_panel_pointer]()
			{
				scroll_panel_pointer->release_key(KEY_UP);
			},
				[scroll_panel_pointer]()
			{
				scroll_panel_pointer->press_key(KEY_UP);
			}, "", "genlight.png", "gendark.png");
			auto down_arrow = make_unique<Button>(960 + 350, 260 + 30, 960 + 370, 260 + 57, "arrow2.png", DECRYPTION_APP,
				[scroll_panel_pointer]()
			{
				scroll_panel_pointer->release_key(KEY_DOWN);
			},
				[scroll_panel_pointer]()
			{
				scroll_panel_pointer->press_key(KEY_DOWN);
			}, "", "genlight.png", "gendark.png");
			children.push_back(scroll_background.get());
			children.push_back(scroll_panel.get());
			children.push_back(scroll_bar.get());
			children.push_back(check_box.get());
			children.push_back(legal_button_ptr);
			children.push_back(up_arrow.get());
			children.push_back(down_arrow.get());
			parent->to_be_added.push_back(std::move(scroll_panel));
			parent->to_be_added.push_back(std::move(scroll_background));
			parent->to_be_added.push_back(std::move(scroll_bar));
			parent->to_be_added.push_back(std::move(check_box));
			parent->to_be_added.push_back(std::move(legal_button));
			parent->to_be_added.push_back(std::move(up_arrow));
			parent->to_be_added.push_back(std::move(down_arrow));
		}, []() {}, "", "invisible.png", "okblockypressed.png");
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
		parent->process_control_changes();
	}
}

void Decryption::draw(TextureManager* texture_manager)
{
	return;
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