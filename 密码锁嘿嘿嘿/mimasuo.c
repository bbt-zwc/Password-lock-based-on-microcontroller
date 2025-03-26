/*********************************************************************************
* ����дʱ�䡿�� 2024��12��16��
* ����    �ߡ��� ����221���ĳ�
* ����    ������ 2.1
* ��ʵ��ƽ̨���� QX-MCS51 ��Ƭ��������
* ���ⲿ���񡿣� 11.0592mhz	
* ������оƬ���� STC89C52
* �����뻷������ Keil ��Visio4
* �������ܡ��� ������			   			            			    
* ��ʹ��˵������ 
**********************************************************************************/
/********************************************************************************
								���ܼ�
				S6---S15	���ּ�0-9					S16---��������							
				S17---����������Ϻ�ȷ��     S18---�������롢�����趨		
				S19---�ر�������							S20---��֤����
				
				��ʼ���룺000000 	����λ����6λ
				ע�⣺�������������ᶪʧ�������ϵ�ʱ������ָ�Ϊԭʼ��000000
				��P1������8λ����LED�������������򿪣�Ϩ�������������

�����ܣ�
		  1��������
		  ���س����ֱ�Ӱ�����S6������������0����8λLED���������򿪣���������ʱ��
		  ��λ�����������ʾС��ܡ�
		  2���������룺
		  ֻ�е�������LED�����󣬸ù��ܷ���ʹ�á�
		  ���Ȱ��¸��������S16��Ȼ��������Ӧ���룬��ʱ��λ����ܻ���ʾ���������Ӧ
		  �����֡������������λ�󣬰���S17ȷ��������ģ��˺������뼴��Ч��
		  3���������룺
		  ����������ʱ�����������¼�S18��������������λ���롣
		  ����������ʱ��������;��������룬Ҳ�ɰ��´˼��������á�
		  4���ر���������
		  ����S19���ɽ��򿪵��������رա�
			ȷ���������---��S19�ر�������---�����µ������������
*******************************************************************************/
#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
#define jingzhen     11059200UL			 /* ʹ��22.1184MHz���� */
#define botelv   9600UL		     /* ����������Ϊ9600 */

uchar old1,old2,old3,old4,old5,old6; //ԭʼ����000000
uchar new1,new2,new3,new4,new5,new6;  //ÿ��MCU�ɼ�������������
uchar a=16,b=16,c=16,d=16,e=16,f=16; //�����������ʾ�ı���
uchar wei,key,temp,confirm;
uchar Number_of_errors=0,i=0;
uint ii;

unsigned char sound1[4] = {0xAA, 0x15, 0x00, 0xBF}; // ����-
unsigned char sound2[4] = {0xAA, 0x14, 0x00, 0xBF}; // ����+
unsigned char end[4] = {0xAA, 0x10, 0x00, 0xBA}; // ����+end
unsigned char song1[6] = {0xAA, 0x07, 0x02, 0x00, 0x01, 0xB4}; // ��1��
unsigned char song2[6] = {0xAA, 0x07, 0x02, 0x00, 0x02, 0xB5}; // ��2��
unsigned char song3[6] = {0xAA, 0x07, 0x02, 0x00, 0x03, 0xB6}; // ��3��
unsigned char song4[6] = {0xAA, 0x07, 0x02, 0x00, 0x04, 0xB7}; // ��4��
unsigned char song5[6] = {0xAA, 0x07, 0x02, 0x00, 0x05, 0xB8}; // ��5��
unsigned char song6[6] = {0xAA, 0x07, 0x02, 0x00, 0x06, 0xB9}; // ��6��
unsigned char song7[6] = {0xAA, 0x07, 0x02, 0x00, 0x07, 0xBA}; // ��7��
unsigned char song8[6] = {0xAA, 0x07, 0x02, 0x00, 0x08, 0xBB}; // ��8��
volatile unsigned char sending;
volatile unsigned char index = 0; // ��ǰ���͵�����

bit allow,genggai,ok,wanbi,retry,close;	 //����״̬λ
sbit S2=P3^0;
sbit S3=P3^1;
sbit S4=P3^2;
sbit S5=P3^3;
sbit dula=P2^6;
sbit wela=P2^7;
sbit beep=P2^3;

void init(void)				// ���ڳ�ʼ��
{
    EA=0; // ��ʱ�ر��ж�
    TMOD &= 0x0F;  // ��ʱ��1������ģʽ2
    TMOD |= 0x20;    // ��ʱ��1������ģʽ2���Զ���װ
    SCON=0x50;     // ���ڹ�����ģʽ1
    TH1=256-jingzhen/(botelv*12*16);  // ���㶨ʱ����װ��ֵ
    TL1=256-jingzhen/(botelv*12*16);
    PCON|=0x80;    // �����ʼӱ�
    ES=1;         // ʹ�ܴ����ж�
    TR1=1;        // ������ʱ��1
    REN=1;        // �������
    EA=1;         // ʹ��ȫ���ж�
}
void send(unsigned char d)		  // ����һ���ֽڵ�����
{
    SBUF=d; // ������д�봮�ڻ�����
    sending=1;	 // ���÷��ͱ�־
    while(sending); // �ȴ��������
}
void uart(void) interrupt 4		 // �����жϷ������
{
    if(RI)    // �����ж�
    {
        RI=0;   // ��������жϱ�־
    }
    else      // �����ж�
    {
        TI=0; // ��������жϱ�־
        sending=0;  // ������ͱ�־
    }
}

unsigned char code table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,
0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x00,0x40};
void Init(void); //��ʼ������
void delay(unsigned char i)
{
	uchar j,k;
  for(j=i;j>0;j--)
    for(k=125;k>0;k--);
}

void display(uchar a,uchar b,uchar c,uchar d,uchar e,uchar f)
{
   dula=0;
   P0=table[a];
   dula=1;
   dula=0;
   
   wela=0;
   P0=0xfe;
   wela=1;
   wela=0;
   delay(5);
   
   P0=table[b];
   dula=1;
   dula=0;
   
   P0=0xfd;
   wela=1;
   wela=0;
   delay(5);

   P0=table[c];
   dula=1;
   dula=0;
   
   P0=0xfb;
   wela=1;
   wela=0;
   delay(5);
   
   P0=table[d];
   dula=1;
   dula=0;
   
   P0=0xf7;
   wela=1;
   wela=0;
   delay(5);
   
   P0=table[e];
   dula=1;
   dula=0;
   
   P0=0xef;
   wela=1;
   wela=0;
   delay(5);
   
   P0=table[f];
   dula=1;
   dula=0;
   
   P0=0xdf;
   wela=1;
   wela=0;
   delay(5);
}


void keyscan()
{
  {	
    P3=0xfe;
    temp=P3;
    temp=temp&0xf0;
    if(temp!=0xf0)
    {
      delay(10);
      if(temp!=0xf0)
      {	
        temp=P3;
        switch(temp)
        {
          case 0xee:
               key=0;
			   wei++;
               break;

          case 0xde:
               key=1;
			   wei++;
               break;

          case 0xbe:
               key=2;
			   wei++;
               break;

          case 0x7e:
               key=3;
			   wei++;
               break;
         }
         while(temp!=0xf0) 
        {
           temp=P3;
           temp=temp&0xf0;
           beep=0;
         }
         beep=1;
      }
    }
    P3=0xfd;
    temp=P3;
    temp=temp&0xf0;
    if(temp!=0xf0)
    {
      delay(10);
      if(temp!=0xf0)
      {
        temp=P3;
        switch(temp)
        {
          case 0xed:
               key=4;
			   wei++;
               break;

          case 0xdd:
               key=5;
			   wei++;
               break;

          case 0xbd:
               key=6;
			   wei++;
               break;

          case 0x7d:
               key=7;
			   wei++;
               break;
         }
         while(temp!=0xf0)
         {
           temp=P3;
           temp=temp&0xf0;
           beep=0;
         }
         beep=1;
      }
      }
    P3=0xfb;
    temp=P3;
    temp=temp&0xf0;
    if(temp!=0xf0)
    {
      delay(10);
      if(temp!=0xf0)
      {
        temp=P3;
        switch(temp)
        {
          case 0xeb:
               key=8;
			   wei++;
               break;

          case 0xdb:
               key=9;
			   wei++;
               break;
			   
          case 0xbb:
               genggai=1;
			   wei=0;
               break;

          case 0x7b:
		  	   if(allow)
               ok=1;
               break;
         }
        while(temp!=0xf0)
         {
           temp=P3;
           temp=temp&0xf0;
           beep=0;
         }
         beep=1;
      }
      }
	  P3=0xf7;
    temp=P3;
    temp=temp&0xf0;
    if(temp!=0xf0)
    {
      delay(10);
      if(temp!=0xf0)
      {
        temp=P3;
        switch(temp)
        {
          case 0xe7:
		  	   retry=1;
               break;

          case 0xd7:
		  	   close=1;
               break;
					case 0xb7:
           confirm=1;
               break;
         }
        while(temp!=0xf0)
         {
           temp=P3;
           temp=temp&0xf0;
           beep=0;
         }
         beep=1;
      }
      }
}
}

void shumima()		//�԰����ɼ��������ݽ��з���
{
	if(!wanbi)
	{
	switch(wei)
	{
		case 1:new1=key; 
			    if(!allow)	a=17;
			   else a=key;	break;
		case 2:new2=key;
				if(a==17) b=17;
				else b=key;	break;
		case 3:new3=key; 
				if(a==17) c=17;
				else c=key;	break;
		case 4:new4=key;
				if(a==17) d=17;
				else d=key;	break;
		case 5:new5=key; 
				if(a==17) e=17;
				else e=key;	break;
		case 6:new6=key; 
				if(a==17) f=17;
				else f=key;
				wanbi=1;	break;
	}
	}
}

void yanzheng()	  //��֤�����Ƿ���ȷ
{
	if(wanbi)	 //ֻ�е���λ�����������Ϻ󷽽�����֤
	{
	if((new1==old1)&(new2==old2)&(new3==old3)&(new4==old4)&(new5==old5)&(new6==old6))
		allow=1;	//�������������ȷ����õ�allowe��һ
	}
}

void main()
{
	init();
	allow=1;
	P1=0x00;
	for(ii=0;ii<=10;ii++)// ��������
	{
	index = 0; // ��������
	while(index < 4) // �����ĸ��ֽڵ�����
                {
                    send(sound1[index]);
                    index++;
										delay(10);
                }
	}
	while(1)
	{
		
		keyscan();
		shumima();
		if(confirm)	 //��֤
		{
			confirm=0;
			ok=0; wei=0;
			yanzheng();
			if(allow)	 //��֤�����allowΪ1������
			{
				P1=0x00;//
				index = 0; // ��������
				while(index < 6)
				{
						send(song1[index]); // ��1��
						index++;
				}
				if(!genggai)
				{wanbi=0;}
				Number_of_errors=0;
			}
			else if(!allow)
			{
				if(!genggai)
				{wanbi=0;}
				
//				for(i=0;i<=10;i++)
//				{
//					beep=~beep;
//					delay(250);
//				}
//				beep=1;
				Number_of_errors++;
				if(Number_of_errors==1)
				{
					index = 0; // ��������
					while(index < 6)
					{
							send(song2[index]); // ��2��
							index++;
					}
				}
				else if(Number_of_errors==2)
				{
					index = 0; // ��������
					while(index < 6)
					{
							send(song3[index]); // ��3��
							index++;
					}
				}
				else if(Number_of_errors>=3)//bo fang yin yue
				{
					index = 0; // ��������
					while(index < 6)
					{
							send(song4[index]); // ��4��
							index++;
					}
//					while(1)
//					{
//					beep=~beep;
//					delay(250);
//					}
			  }
			}
		}
		if(genggai)	  //��S16��������������£�genggai�ᱻ��һ
		{
			if(allow)	 //���Ѿ������򿪣����и��������Ȩ��
			{
				while(!wanbi)	//���µ���λ����û���趨�꣬��һֱ������ѭ��
				{
				 	keyscan();
					shumima();
					if(retry|close)	 //����̽�⵽���Լ�S18���߹ر���������S19������ʱ��������
					{	wanbi=1;
						break;
					}
					display(a,b,c,d,e,f);
				}
			}
		}
		if(ok)	  //��������ʱ����������λ�������������ʱ�����԰��´˼��������������
		{		  //����ʱ�䰴�´˼���Ч
			ok=0; wei=0;
			genggai=0;
			old1=new1;old2=new2;old3=new3; //��ʱ���ɵ����뽫������
			old4=new4;old5=new5;old6=new6;
			a=16;b=16;c=16;d=16;e=16;f=16;
		}
		if(retry)	//�����԰���S18�����£�retry�ᱻ��λ
		{
		retry=0; wei=0;wanbi=0;
		a=16;b=16;c=16;d=16;e=16;f=16;
		new1=0;new2=0;new3=0;new4=0;new5=0;new6=0;		
		}
		if(close)  //���ر����������������£�close�ᱻ��λ
		{
			close=0;genggai=0;//���б����������㡣
			wei=0;	wanbi=0;
			allow=0;
			P1=0xff;
			a=16;b=16;c=16;d=16;e=16;f=16;
			new1=0;new2=0;new3=0;new4=0;new5=0;new6=0;
		}
		display(a,b,c,d,e,f); //ʵʱ��ʾ
	}
}
