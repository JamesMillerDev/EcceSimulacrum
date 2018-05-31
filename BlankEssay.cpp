#include "BlankEssay.h"
#include "DraggableWord.h"

BlankEssay::BlankEssay(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
}

void BlankEssay::init()
{
	blanks.push_back(ScreenElement(143 + x1, 145 + y1, 143 + 34 + x1, 145 + 34 + y1, "testblue.png"));
	blanks.push_back(ScreenElement(275 + x1, 243 + y1, 275 + 34 + x1, 243 + 34 + y1, "testred.png"));
	auto word1 = make_unique<DraggableWord>(x1 + 50, y1 + 50, x1 + 50 + 34, y1 + 50 + 34, "testblue.png", this);
	auto word2 = make_unique<DraggableWord>(x1 + 50, y1 + 100, x1 + 50 + 34, y1 + 100 + 34, "testred.png", this);
	words.push_back(word1.get());
	words.push_back(word2.get());
	parent->to_be_added.push_back(std::move(word1));
	parent->to_be_added.push_back(std::move(word2));
}

void BlankEssay::animate()
{
	if (check_alpha > 0.0)
		check_alpha -= 0.02;

	if (check_alpha <= 0.0)
		check_alpha = 0.0;
}

void BlankEssay::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	if (check_alpha == 0.0)
		return;

	ScreenElement check(0, 0, 50, 50, "greencheck.png");
	check.center_in(this);
	glColor4f(1.0, 1.0, 1.0, check_alpha);
	check.draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

bool BlankEssay::essay_complete()
{
	for (int i = 0; i < words.size(); ++i)
	{
		if (words[i]->movable)
			return false;
	}

	return true;
}

void BlankEssay::word_in_blank(DraggableWord* word)
{
	for (int i = 0; i < blanks.size(); ++i)
	{
		if (blanks[i].name == word->name)
		{
			if (intersects(blanks[i], *word))
			{
				word->movable = false;
				word->x1 = blanks[i].x1;
				word->x2 = blanks[i].x2;
				word->y1 = blanks[i].y1;
				word->y2 = blanks[i].y2;
				parent->sound_manager->play_sound("ding.wav");
				check_alpha = 1.0;
			}

			break;
		}
	}
}

BlankEssay::~BlankEssay()
{
	for (int i = 0; i < words.size(); ++i)
		words[i]->marked_for_deletion = true;
}