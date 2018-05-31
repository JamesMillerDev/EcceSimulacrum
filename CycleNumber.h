#ifndef CYCLE_NUMBER
#define CYCLE_NUMBER

struct CycleNumber
{
	float current_value, start, end, inc;
	bool towards;
	CycleNumber() : current_value(0), start(0), end(0), inc(0) {}
	CycleNumber(float s, float e, float i)
	{
		current_value = s;
		start = s;
		end = e;
		inc = i;
		towards = true;
	}

	void restart()
	{
		current_value = start;
		towards = true;
	}

	void step()
	{
		if (towards)
		{
			current_value += inc;
			if (current_value >= end)
				towards = false;
		}

		else
		{
			if (current_value <= start)
				return;

			current_value -= inc;
		}
	}
};

#endif