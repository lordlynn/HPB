
#ifndef CAN_H_
#define CAN_H_

#define NODE_A 1

void FLEXCAN0_init(void);
void FLEXCAN0_transmit_msg(void);
void FLEXCAN0_receive_msg(void);
void port_init(void);

#endif /* CAN_H_ */
