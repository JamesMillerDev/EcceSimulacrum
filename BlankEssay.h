#ifndef BLANK_ESSAY
#define BLANK_ESSAY

#include "Computer.h"

struct DraggableWord;

struct BlankEssay : public ScreenElement
{
	BlankEssay(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent);
	void init();
	vector<ScreenElement> blanks;
	vector<DraggableWord*> words;
	bool essay_complete();
	void word_in_blank(DraggableWord* word);
	Computer* parent;
	float check_alpha = 0.0;
	void draw(TextureManager* texture_manager);
	void animate();
	~BlankEssay();
};

#endif