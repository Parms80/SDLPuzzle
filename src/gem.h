#ifndef GEM_H
#define GEM_H

struct gem
{
	float x;
	float y;
	float velocity;
	int type;
	int state;
	int row;
	int column;
	int prevState;
	int dropToRow;
};

#endif