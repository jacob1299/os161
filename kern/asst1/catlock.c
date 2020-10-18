/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2


/*
 *Global vars
 *
 */

struct lock *cat_mouse_lock, *bowl_lock[NFOODBOWLS], *finished_count_lock;
int cats_currently_eating = 0;
int num_finished = 0;



/*
 * 
 * Function Definitions
 * 
 */

static void 
lock_eat(const char *who, int num, int bowl, int iteration) 
{
	kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
		bowl, iteration);
	clocksleep(1);
	kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
		bowl, iteration);

}

static void try_to_eat_from_bowl(const char *who, int num, int iteration) 
{
	int bowl = (int)(random() % NFOODBOWLS) + 1; //create bowl int to lock bowls

	lock_acquire(bowl_lock[bowl]); //acquire lock on bowl
	lock_eat(who, num, bowl, iteration); //eat from wherever lets eat
	lock_release(bowl_lock[bowl]); //release lock on bowl

}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;

	int i;
	for(i = 0; i<4;) 
	{
		lock_acquire(cat_mouse_lock);

		if((cats_currently_eating == 0) || (cats_currently_eating == 1)) 
		{
		     	cats_currently_eating++; //increment cats eating
			lock_release(cat_mouse_lock); //release lock
			try_to_eat_from_bowl("cat", catnumber, i); //try to eat from a bowl

			lock_acquire(cat_mouse_lock); //acquire lock
			cats_currently_eating--; //decrement cats eating
			lock_release(cat_mouse_lock); //release cat_mouse_lock
	
			++i;

		}
		else 
		{
			lock_release(cat_mouse_lock);

		}
	thread_yield();
	}
	
	lock_acquire(finished_count_lock);
	++num_finished;//increment number finished
	lock_release(finished_count_lock);
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;

	int i;

	for(i = 0; i<4;) 
	{
		lock_acquire(cat_mouse_lock); //acquire cat_mouse_lock

		if(cats_currently_eating == 0 || cats_currently_eating >= 3) //only do if no cats eating OR if more than 2 are eating 
		{
			cats_currently_eating = (cats_currently_eating == 0) ? 3 : (cats_currently_eating + 1); //if cats eating is 0, set to 3, else increment. This resets the counter
			lock_release(cat_mouse_lock); //release cat_mouse_lock
			try_to_eat_from_bowl("mouse", mousenumber, i); //let mouse attempt to eat

			lock_acquire(cat_mouse_lock); //acquire lock for cat_mouse_lock
			cats_currently_eating = (cats_currently_eating == 3) ? 0 : (cats_currently_eating - 1); //if cats eating is 3, set to 0, else decrement. This fixes queue
			lock_release(cat_mouse_lock); //release lock 

			++i;

		}
		else 
		{
			lock_release(cat_mouse_lock);
		}
		thread_yield();
	}	
	lock_acquire(finished_count_lock); //acquire finished_count_lock
	++num_finished; //increment number finished
	lock_release(finished_count_lock); //release finished_count_lock
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
   
        /*
         * Start NCATS catlock() threads.
         */
	
	cat_mouse_lock = lock_create("cat_mouse_lock"); //create cat_mouse_lock
	assert(cat_mouse_lock != NULL); //make sure is not NULL
	finished_count_lock = lock_create("finished count lock"); //create finished count lock
	assert(finished_count_lock != NULL); //make sure is not NULL
	for(index = 0; index < NFOODBOWLS; ++index) { //loop through creating locks on bowls and making sure not NULL
		bowl_lock[index] = lock_create("bowl_lock");
		assert(bowl_lock[index] != NULL);
	}
	
	cats_currently_eating = 0; //set vars
	num_finished = 0;


        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

	while(num_finished < (NCATS + NMICE)) {
		thread_yield();
	}
	
	//DESTROY LOCKS
	lock_destroy(cat_mouse_lock);
	lock_destroy(finished_count_lock);
	for(index = 0; index < NFOODBOWLS; ++index) {
		lock_destroy(bowl_lock[index]);
	}

        return 0;
}

/*
 * End of catlock.c
 */
