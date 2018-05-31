#include <algorithm>
#include <iomanip>
#include <sstream>
#include "ShoppingCenter.h"
#include "ScrollBar.h"
#include "Button.h"
#include "CircleAnimation.h"
#include "FadingObject.h"
#include "keymappings.h"

string ShoppingCenter::two_decimals(float num)
{
	stringstream ss;
	ss << fixed << setprecision(2) << num;
	return ss.str();
}

void ShoppingCenter::remove_children()
{
	for (int i = 0; i < children.size(); ++i)
		children[i]->marked_for_deletion = true;

	children.clear();
	if (clear_messages)
	{
		message_queue.clear();
		clear_messages = false;
		messages_button->name = "messagesbuttondesat.png";
		messages_button->sheen_name = "invisible.png";
		messages_button->pressed_name = "invisible.png";
		messages_button->release_function = []() {};
	}
}

float ShoppingCenter::get_total()
{
	float total = 0;
	for (int i = 0; i < cart.size(); ++i)
		total += cart[i].price;

	return total;
}

void ShoppingCenter::go_to_cart()
{
	bool empty = this->cart.empty();
	this->remove_children();
	this->scroll_panel_ptr->change_website("cart");
	auto buybutton = make_unique<Button>(500, 100, 500 + 188, 100 + 46, empty ? "buybuttondesat.png" : "buybutton.png", VIDEO_APP,
		[this]()
	{
		this->remove_children();
		this->scroll_panel_ptr->change_website("confirmation");
		auto warning_message = make_unique<InfoText>(300, 700, 300, 700, "invisible.png", "Are you sure you want to purchase?  This action will decrease your available funds.", VIDEO_APP);
		if (store_number == 2 && !item_in_cart("Baby"))
		{
			cart.push_back(ShoppingItem("baby", "Baby", 100000, { "Medium-High", "Medium-Low", "YES", "NO" }));
			baby_insertion_count++;
			if (baby_insertion_count == 3)
				add_message(2);
		}

		auto confirmbutton = make_unique<Button>(300, 500, -1, -1, "confirmbutton.png", VIDEO_APP, [this]() 
		{
			if (get_total() > this->funds)
			{
				this->remove_children();
				this->scroll_panel_ptr->change_website("errorpurchase");
				auto error_message = make_unique<InfoText>(300, 700, 300, 700, "invisible.png", "You do not have sufficient funds to complete your purchase.", VIDEO_APP);
				this->children.push_back(error_message.get());
				parent->screen_elements.insert(parent->screen_elements.end(), std::move(error_message));
			}

			else
			{
				this->remove_children();
				this->scroll_panel_ptr->change_website("thankyou");
				auto thankyoumessage = make_unique<InfoText>(300, 700, 300, 700, "invisible.png", "Thank you for your purchase!", VIDEO_APP);
				this->children.push_back(thankyoumessage.get());
				parent->screen_elements.insert(parent->screen_elements.end(), std::move(thankyoumessage));
				change_funds(-1 * get_total());
				for (int i = 0; i < cart.size(); ++i)
					purchased.push_back(cart[i]);

				cart.clear();
				buy_button_check();
				if (!made_purchase)
				{
					made_purchase = true;
					add_message(1);
				}
			}
		}, []() {}, "", "genlight.png", "gendark.png");
		auto hesitatebutton = make_unique<Button>(600, 500, -1, -1, "hesitatebutton.png", VIDEO_APP, [this]()
		{
			this->go_to_cart();
		}, []() {}, "", "genlight.png", "gendark.png");
		this->children.push_back(warning_message.get());
		this->children.push_back(confirmbutton.get());
		this->children.push_back(hesitatebutton.get());
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(warning_message));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(confirmbutton));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(hesitatebutton));
	}, []() {}, "", empty ? "invisible.png" : "genlight.png", empty ? "invisible.png" : "gendark.png");
	if (empty)
		buybutton->release_function = []() {};

	this->children.push_back(buybutton.get());
	this->parent->screen_elements.insert(parent->screen_elements.end(), std::move(buybutton));
}

void ShoppingCenter::add_message(int number)
{
	messages_button->name = "messagesbutton.png";
	messages_button->sheen_name = "genlight.png";
	messages_button->pressed_name = "gendark.png";
	messages_button->release_function = [this, number]()
	{
		if (number == 2 && funds < 100000)
			change_funds(100000);

		this->remove_children();
		this->scroll_panel_ptr->change_website("messages");
		this->clear_messages = true;
		if (this->messages_number != NULL)
		{
			this->messages_number->marked_for_deletion = true;
			this->messages_number = NULL;
		}

		ownership_button->name = "ownershipbutton.png";
		ownership_button->sheen_name = "genlight.png";
		ownership_button->pressed_name = "gendark.png";
		ownership_button->release_function = [this]()
		{
			this->remove_children();
			this->scroll_panel_ptr->change_website("ownership");
			this->visited_ownership = true;
		};
	};

	this->parent->screen_elements.insert(this->parent->screen_elements.end(), make_unique<CircleAnimation>(480, 910, 0, 0, this->parent));
	auto number_text = make_unique<ScreenElement>(475, 914, -1, -1, "messages1.png", VIDEO_APP);
	messages_number = number_text.get();
	this->parent->screen_elements.insert(this->parent->screen_elements.end(), std::move(number_text));
	message_queue.push_back("message" + to_string(number) + ".png");
}

void ShoppingCenter::change_funds(float delta)
{
	funds += delta;
	string sign = delta < 0 ? "-" : "+";
	auto change = make_unique<InfoText>(250, 140, 250, 140, "invisible.png", sign + "$" + two_decimals(delta), VIDEO_APP);
	lost_money = change.get();
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(change));
	wallet_text->text = "Wallet: $" + two_decimals(funds);
}

void ShoppingCenter::animate()
{
	if (lost_money != NULL)
	{
		lost_money->translate(0, 3);
		lost_money->alpha -= 0.05;
		if (lost_money->alpha <= 0)
		{
			lost_money->marked_for_deletion = true;
			lost_money = NULL;
			parent->process_control_changes();
		}
	}
	
	if (loading)
	{
		if (frames == 0)
		{
			auto cover = make_unique<ScreenElement>(0, 1080 - 500, 500, 1080, "black.png", VIDEO_APP);
			loading_cover = cover.get();
			parent->screen_elements.insert(parent->screen_elements.end(), std::move(cover));
		}

		if (frames == 10)
		{
			loading_cover->y1 = 0;
			loading_cover->x2 = 1920;
			parent->close_for_decrypt();
			parent->decrypt_forcing = false;
		}

		if (frames == 90)
		{
			loading_cover->name = "white.png";
			parent->screen_elements.insert(parent->screen_elements.end() - 1, make_unique<ScreenElement>(0, 0, 1920, 1080, "clouds.png", VIDEO_APP));
		}

		if (frames > 90)
		{
			loading_cover->alpha -= 0.05;
			if (loading_cover->alpha <= 0.0)
			{
				parent->close_application(VIDEO_APP);
				parent->process_control_changes();
				return;
				loading_cover->marked_for_deletion = true;
				loading = false;
			}
		}

		frames++;
		return;
	}
	
	if (never_animated)
	{
		never_animated = false;
		auto store1 = make_unique<ShimmeringText>(500, 500, 900, 600, "store1.png", VIDEO_APP);
		store1->center();
		title_text = store1.get();
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(store1));
	}

	if (alt_title_text != NULL && alt_title_text->frames >= 60 * 8)
	{
		alt_title_text->marked_for_deletion = true;
		alt_title_text = NULL;
		parent->process_control_changes();
	}

	if (title_text != NULL && title_text->frames >= 60 * 8)
	{
		auto clouds = make_unique<ScreenElement>(0, 0, 1920, 1080, "clouds.png", VIDEO_APP);
		cover = clouds.get();
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<ScreenElement>(0, 0, 1920, 1080, "storebackground.png", VIDEO_APP));
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<ScreenElement>(258, 888, 1661, 970, "metalholder.png", VIDEO_APP));
		auto scroll_panel = make_unique<ScrollPanel>(306, 178, 1660, 868, parent, VIDEO_APP, true);
		scroll_panel_ptr = scroll_panel.get();
		auto scroll_bar = make_unique<ScrollBar>(1619 - 17, 300.0, 1619 + 17, 850, "bluescrollbar.png", VIDEO_APP, scroll_panel.get());
		scroll_panel->scroll_bar = scroll_bar.get();
		auto scroll_panel_pointer = scroll_panel.get();
		scroll_bar->min_y = 178.0;
		scroll_bar->max_y = 868.0;
		scroll_panel->change_website("decryptpage");
		//up_arrow->surrender_focus_on_up_click = true; //TODO do this for every scroll bar TODO this isn't actually the fix, fix it for real
		auto scroll_background = make_unique<ScreenElement>(1619, 178, 1620, 868, "white.png", VIDEO_APP);
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(scroll_panel));
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<ScreenElement>(271, 839, -1, -1, "fadeout1.png", VIDEO_APP));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(scroll_background));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(scroll_bar));
		populate_inventory();
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<Button>(260, 900, -1, -1, "cartbutton.png", VIDEO_APP, [this]()
		{
			this->remove_children();
			this->go_to_cart();
			exit_logic();
		}, []() {}, "", "genlight.png", "gendark.png"));
		auto mbutton = make_unique<Button>(460, 900, -1, -1, "messagesbuttondesat.png", VIDEO_APP, [this]() {}, []() {}, "", "invisible.png", "invisible.png");
		messages_button = mbutton.get();
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(mbutton));
		auto obutton = make_unique<Button>(660, 900, -1, -1, "ownershipbuttondesat.png", VIDEO_APP, []() {}, []() {});
		ownership_button = obutton.get();
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(obutton));
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<Button>(860, 900, -1, -1, "cataloguebutton.png", VIDEO_APP, [this]()
		{
			//this->go_to_dispute();
			//return;
			this->remove_children();
			this->scroll_panel_ptr->change_website("catalogue");
			exit_logic();
		}, []() {}, "", "genlight.png", "gendark.png"));
		parent->screen_elements.insert(parent->screen_elements.end(), make_unique<ScreenElement>(260, 165, 1660, 166, "black.png", VIDEO_APP));
		auto wallet = make_unique<InfoText>(250, 140, 250, 140, "invisible.png", "Wallet: $" + two_decimals(funds), VIDEO_APP);
		wallet_text = wallet.get();
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(wallet));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(clouds));
		title_text->marked_for_deletion = true;
		title_text = NULL;
		parent->to_be_added.clear(); //TODO why is this needed?
		scroll_panel_ptr->text_field_ptr = dummy_text_field.get();
		scroll_panel_ptr->shopping_center = this;
		scroll_panel_ptr->change_website("catalogue");
		parent->process_control_changes();
	}

	if (cover != NULL && alt_title_text == NULL)
	{
		if (cover->alpha > 0.0)
			cover->alpha -= 0.02;

		else
		{
			cover->marked_for_deletion = true;
			cover = NULL;
			parent->process_control_changes();
		}
	}
}

bool ShoppingCenter::item_in_cart(string name)
{
	for (int i = 0; i < cart.size(); ++i)
	{
		if (cart[i].name == name)
			return true;
	}

	return false;
}

bool ShoppingCenter::is_purchased(string name)
{
	for (int i = 0; i < purchased.size(); ++i)
	{
		if (purchased[i].name == name)
			return true;
	}

	return false;
}

void ShoppingCenter::remove_item_from_cart(string name)
{
	cart.erase(remove_if(cart.begin(), cart.end(), [name](ShoppingItem item) {return item.name == name;}), cart.end());
	buy_button_check();
}

void ShoppingCenter::buy_button_check()
{
	if (cart.empty())
	{
		for (int i = 0; i < children.size(); ++i)
		{
			if (children[i]->name == "buybutton.png")
			{
				Button* buybutton = dynamic_cast<Button*>(children[i]);
				buybutton->name = "buybuttondesat.png";
				buybutton->pressed_name = "invisible.png";
				buybutton->sheen_name = "invisible.png";
				buybutton->release_function = []() {};
			}
		}
	}
}

void ShoppingCenter::exit_logic()
{
	if (store_number == 1 && visited_ownership)
		change_store();
}

void ShoppingCenter::change_store()
{
	store_number++;
	auto clouds = make_unique<ScreenElement>(0, 0, 1920, 1080, "clouds.png", VIDEO_APP);
	cover = clouds.get();
	auto store = make_unique<ShimmeringText>(500, 500, 900, 600, "store" + to_string(store_number) + ".png", VIDEO_APP);
	store->center();
	alt_title_text = store.get();
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(clouds));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(store));
	inventory.clear();
	populate_inventory();
	cart.clear();
	this->remove_children();
	this->scroll_panel_ptr->change_website("cart");
	this->scroll_panel_ptr->change_website("catalogue");
}

void ShoppingCenter::go_to_dispute()
{
	this->remove_children();
	auto clouds = make_unique<ScreenElement>(0, 0, 1920, 1080, "clouds.png", VIDEO_APP);
	cover = clouds.get();
	auto dispute_text = make_unique<ShimmeringText>(0, 0, 772, 65, "disputetext.png", VIDEO_APP);
	dispute_text->center();
	alt_title_text = dispute_text.get();
	parent->screen_elements.insert(parent->screen_elements.end(), make_unique<ScreenElement>(0, 0, 1920, 1080, "disputebg.png", VIDEO_APP)); //probably need to save reference to this somewhere
	auto dispute1 = make_unique<ScreenElement>(294, 1080 - 266, -1, -1, "dispute1.png", VIDEO_APP);
	children.push_back(dispute1.get());
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(dispute1));
	auto nbutton = make_unique<Button>(1425, 1080 - 257, -1, -1, "nextbutton.png", VIDEO_APP, [this]()
	{
		this->remove_children();
		auto dispute2 = make_unique<ScreenElement>(294, 1080 - 266, -1, -1, "dispute2.png", VIDEO_APP);
		this->children.push_back(dispute2.get());
		this->parent->screen_elements.push_back(std::move(dispute2));
		auto os1 = make_unique<Button>(310, 700, 710, 746, "os1n.png", VIDEO_APP, [this]() {this->parent->sound_manager->play_sound("ding.wav"); this->parent->screen_elements.push_back(make_unique<FadingObject>(310, 700, 360, 750, "greencheck.png", this->parent));}, []() {}, "", "os1h.png", "os1h.png");
		this->children.push_back(os1.get());
		this->parent->screen_elements.push_back(std::move(os1));

		auto os2 = make_unique<Button>(310, 600, 710, 646, "os2n.png", VIDEO_APP, [this]() {this->parent->sound_manager->play_sound("buzzer.wav"); this->parent->screen_elements.push_back(make_unique<FadingObject>(310, 600, 360, 650, "redx.png", this->parent));}, []() {}, "", "os2h.png", "os2h.png");
		this->children.push_back(os2.get());
		this->parent->screen_elements.push_back(std::move(os2));

		auto os3 = make_unique<Button>(310, 500, 710, 546, "os3n.png", VIDEO_APP, [this]() {this->parent->sound_manager->play_sound("buzzer.wav"); this->parent->screen_elements.push_back(make_unique<FadingObject>(310, 500, 360, 550, "redx.png", this->parent));}, []() {}, "", "os3h.png", "os3h.png");
		this->children.push_back(os3.get());
		this->parent->screen_elements.push_back(std::move(os3));

		auto os4 = make_unique<Button>(310, 400, 710, 446, "os4n.png", VIDEO_APP, [this]() {this->parent->sound_manager->play_sound("buzzer.wav"); this->parent->screen_elements.push_back(make_unique<FadingObject>(310, 400, 360, 450, "redx.png", this->parent));}, []() {}, "", "os4h.png", "os4h.png");
		this->children.push_back(os4.get());
		this->parent->screen_elements.push_back(std::move(os4));
	}, []() {});
	children.push_back(nbutton.get());
	parent->screen_elements.push_back(std::move(nbutton));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(clouds));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(dispute_text));
	inventory.clear();
	cart.clear();
}

void ShoppingCenter::populate_inventory()
{
	if (store_number == 1)
	{
		inventory.push_back(ShoppingItem("flowers.png", "Flowers", 100, {"Medium-High", "Medium-Low", "YES", "NO"}));
		inventory.push_back(ShoppingItem("chair.png", "Chair", 2000, { "Medium-High", "Medium-Low", "YES", "NO" }));
		inventory.push_back(ShoppingItem("extinguisher.png", "•Extinguisher", 500, { "Medium-High", "Medium-Low", "YES", "NO" }));
		inventory.push_back(ShoppingItem("blueblock.png", "Blue Block", 300, { "Medium-High", "Medium-Low", "YES", "NO" }));
		inventory.push_back(ShoppingItem("lockedfolder.png", "Locked Folder", 5000, { "Medium-High", "Medium-Low", "YES", "NO" }));
	}

	else if (store_number == 2)
	{
		inventory.push_back(ShoppingItem("alien.png", "Alien", 3000, { "Medium-High", "Medium-Low", "YES", "NO" }));
		inventory.push_back(ShoppingItem("baby", "Baby", 100000, { "Medium-High", "Medium-Low", "YES", "NO" }));
	}
}