#line 1 "Blink.ino"
  
       
                                                                         

                                                                    
                                                                   
                                                                   
                                        

                                            

                     
                     
   


                                                                       
#include "Arduino.h"
#include "vm.h"
#include "HAL.h"
#include <inttypes.h>

#include <movement.h>

void setup();
void loop();
#line 18
void setup() {
	serial_configure(9600);
	while (1)
	{
		printnum(TCNT0);
	}
	/*setup_movement();
	ahead_R();
	while (read_encoder_counter(RIGHT) < 60)
	{
		printnum(read_encoder_time(RIGHT));
		print("\n");
	}*/
	stop_motor_R();
	uint8_t size1 = read_byte();
	uint8_t size2 = read_byte();
	// send_byte(size1);
	// send_byte(size2);
	uint16_t tot_size = (uint16_t)size1 | ((uint16_t)size2 << 8);
	int i;

	for (i = 0; i < 4*tot_size; i++)
	{
		VM_memory[i] = read_byte();
		//send_byte(VM_memory[i]);
	}

#if MEASURING
	unsigned long tic = micros();
#endif
#if RUN_VM
	vm_cpu();
#endif
#if MEASURING
	unsigned long toc = micros();
	print("TIMESPENT.");
	printnum(toc - tic);
	print("\n");
#endif
	send_byte('\0');
}

                                                     
void loop() {

}