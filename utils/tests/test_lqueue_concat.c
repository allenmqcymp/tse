/* test_queue_concat.c --- 
 * 
 * 
 * Author: Steven J. Signorelli Jr. 
 * Description: tests the concat function of a lqueue
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "time.h"
#include "list.h"
#include "lqueue.h"


int main(void){
	car_t car1;
	strcpy(car1.plate, "aaa111");
	car1.price = 100.0;
	
	car_t car2;
	strcpy(car2.plate, "bbb222");
	car2.price = 200.0;

	car_t car3;
	strcpy(car3.plate, "ccc333");
	car3.price = 300.0;

	lqueue_t *qp1 = lqopen();
	lqueue_t *qp2 = lqopen();

	lqput(qp1, &car1);
	lqput(qp1, &car2);
	lqput(qp2, &car3);

	lqconcat(qp1, qp2);

  	car_t *car12 = (car_t *)lqget(qp1);
	car_t *car22 = (car_t *)lqget(qp1);
	car_t *car32 = (car_t *)lqget(qp1);

	if((car1.price != car12->price) || (car2.price != car22->price) || (car3.price != car32->price)){
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);


}
