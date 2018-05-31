#ifndef SHOPPING_CENTER
#define SHOPPING_CENTER

#include "Computer.h"
#include "ScrollPanel.h"
#include "ShimmeringText.h"
#include "TextField.h"
#include "Button.h"

struct ShoppingCenter : public ScreenElement
{
	unique_ptr<TextField> dummy_text_field;
	Computer* parent;
	bool never_animated;
	bool made_purchase;
	bool visited_ownership;
	ScreenElement* cover;
	ShimmeringText* title_text;
	ShimmeringText* alt_title_text;
	ScrollPanel* scroll_panel_ptr;
	ScreenElement* loading_cover;
	vector<ShoppingItem> inventory;
	vector<ShoppingItem> cart;
	vector<ShoppingItem> purchased;
	string current_item;
	int frames;
	bool loading;
	bool clear_messages;
	float funds;
	InfoText* wallet_text;
	vector<ScreenElement*> children;
	InfoText* lost_money;
	ScreenElement* messages_number;
	vector<string> message_queue;
	Button* messages_button;
	Button* ownership_button;
	Button* next_button;
	int dispute_frame = 0;
	int store_number;
	int baby_insertion_count;
	ShoppingCenter(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
	{
		never_animated = true;
		made_purchase = false;
		cover = NULL;
		title_text = NULL;
		dummy_text_field = make_unique<TextField>(0, 0, 100, 100, "textfield.png", parent, VIDEO_APP);
		frames = 0;
		loading = true;
		funds = 5000;
		lost_money = NULL;
		clear_messages = false;
		visited_ownership = false;
		store_number = 1;
		alt_title_text = NULL;
		baby_insertion_count = 0;
	}
	
	void animate();
	bool item_in_cart(string name);
	void remove_item_from_cart(string name);
	void remove_children();
	void go_to_cart();
	float get_total();
	string two_decimals(float num);
	void change_funds(float delta);
	bool is_purchased(string name);
	void buy_button_check();
	void exit_logic();
	void change_store();
	void populate_inventory();
	void add_message(int number);
	void go_to_dispute();
};

#endif