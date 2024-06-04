#ifndef NEOTIMER_H
#define NEOTIMER_H

#define NEOTIMER_INDEFINITE -1
#define NEOTIMER_UNLIMITED -1


#include <Arduino.h>

class Neotimer{
	public:
	//Methods
	Neotimer();
	Neotimer(unsigned long _t);      //Constructor
	~Neotimer();            //Destructor

	void init();            //Initializations
	bool done();         //Indicates time has elapsed
	bool repeat(int times);
	bool repeat(int times, unsigned long _t);
	bool repeat();
	void repeatReset();
	bool waiting();		// Indicates timer is started but not finished
	bool started();		// Indicates timer has started
	void start();			//Starts a timer
	unsigned long stop();	//Stops a timer and returns elapsed time
	unsigned long getEllapsed();	// Gets the ellapsed time
	void restart();
	void reset();           //Resets timer to zero
	void set(unsigned long t);
	unsigned long get();
	bool debounce(bool signal);
	int repetitions = NEOTIMER_UNLIMITED;

	private:

	struct myTimer{
		unsigned long time;
		unsigned long last;
		bool done;
		bool started;
	};

	struct myTimer _timer;
	bool _waiting;
};
#endif
