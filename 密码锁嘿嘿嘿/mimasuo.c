/*********************************************************************************
* 【编写时间】： 2024年12月16日
* 【作    者】： 电子221赵文晨
* 【版    本】： 2.1
* 【实验平台】： QX-MCS51 单片机开发板
* 【外部晶振】： 11.0592mhz	
* 【主控芯片】： STC89C52
* 【编译环境】： Keil μVisio4
* 【程序功能】： 密码锁			   			            			    
* 【使用说明】： 
**********************************************************************************/
/********************************************************************************
								功能键
				S6---S15	数字键0-9					S16---更改密码							
				S17---更改密码完毕后确认     S18---重试密码、重新设定		
				S19---关闭密码锁							S20---验证密码
				
				初始密码：000000 	密码位数：6位
				注意：掉电后，所设密码会丢失，重新上点时，密码恢复为原始的000000
				与P1相连的8位发光LED点亮代表锁被打开；熄灭代表锁被锁上

程序功能：
		  1、开锁：
		  下载程序后，直接按六次S6（即代表数字0），8位LED亮，锁被打开，输入密码时，
		  六位数码管依次显示小横杠。
		  2、更改密码：
		  只有当开锁（LED亮）后，该功能方可使用。
		  首先按下更改密码键S16，然后设置相应密码，此时六位数码管会显示设置密码对应
		  的数字。最后设置完六位后，按下S17确认密码更改，此后新密码即生效。
		  3、重试密码：
		  当输入密码时，密码输错后按下键S18，可重新输入六位密码。
		  当设置密码时，设置中途想更改密码，也可按下此键重新设置。
		  4、关闭密码锁：
		  按下S19即可将打开的密码锁关闭。
			确认密码更改---按S19关闭密码锁---输入新的密码打开密码锁
*******************************************************************************/
#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int
#define jingzhen     11059200UL			 /* 使用22.1184MHz晶振 */
#define botelv   9600UL		     /* 波特率设置为9600 */

uchar old1,old2,old3,old4,old5,old6; //原始密码000000
uchar new1,new2,new3,new4,new5,new6;  //每次MCU采集到的密码输入
uchar a=16,b=16,c=16,d=16,e=16,f=16; //送入数码管显示的变量
uchar wei,key,temp,confirm;
uchar Number_of_errors=0,i=0;
uint ii;

unsigned char sound1[4] = {0xAA, 0x15, 0x00, 0xBF}; // 音量-
unsigned char sound2[4] = {0xAA, 0x14, 0x00, 0xBF}; // 音量+
unsigned char end[4] = {0xAA, 0x10, 0x00, 0xBA}; // 音量+end
unsigned char song1[6] = {0xAA, 0x07, 0x02, 0x00, 0x01, 0xB4}; // 第1首
unsigned char song2[6] = {0xAA, 0x07, 0x02, 0x00, 0x02, 0xB5}; // 第2首
unsigned char song3[6] = {0xAA, 0x07, 0x02, 0x00, 0x03, 0xB6}; // 第3首
unsigned char song4[6] = {0xAA, 0x07, 0x02, 0x00, 0x04, 0xB7}; // 第4首
unsigned char song5[6] = {0xAA, 0x07, 0x02, 0x00, 0x05, 0xB8}; // 第5首
unsigned char song6[6] = {0xAA, 0x07, 0x02, 0x00, 0x06, 0xB9}; // 第6首
unsigned char song7[6] = {0xAA, 0x07, 0x02, 0x00, 0x07, 0xBA}; // 第7首
unsigned char song8[6] = {0xAA, 0x07, 0x02, 0x00, 0x08, 0xBB}; // 第8首
volatile unsigned char sending;
volatile unsigned char index = 0; // 当前发送的索引

bit allow,genggai,ok,wanbi,retry,close;	 //各个状态位
sbit S2=P3^0;
sbit S3=P3^1;
sbit S4=P3^2;
sbit S5=P3^3;
sbit dula=P2^6;
sbit wela=P2^7;
sbit beep=P2^3;

void init(void)				// 串口初始化
{
    EA=0; // 暂时关闭中断
    TMOD &= 0x0F;  // 定时器1工作在模式2
    TMOD |= 0x20;    // 定时器1工作在模式2，自动重装
    SCON=0x50;     // 串口工作在模式1
    TH1=256-jingzhen/(botelv*12*16);  // 计算定时器重装载值
    TL1=256-jingzhen/(botelv*12*16);
    PCON|=0x80;    // 波特率加倍
    ES=1;         // 使能串口中断
    TR1=1;        // 启动定时器1
    REN=1;        // 允许接收
    EA=1;         // 使能全局中断
}
void send(unsigned char d)		  // 发送一个字节的数据
{
    SBUF=d; // 将数据写入串口缓冲区
    sending=1;	 // 设置发送标志
    while(sending); // 等待发送完成
}
void uart(void) interrupt 4		 // 串口中断服务程序
{
    if(RI)    // 接收中断
    {
        RI=0;   // 清除接收中断标志
    }
    else      // 发送中断
    {
        TI=0; // 清除发送中断标志
        sending=0;  // 清除发送标志
    }
}

unsigned char code table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,
0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x00,0x40};
void Init(void); //初始化函数
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

void shumima()		//对按键采集来的数据进行分配
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

void yanzheng()	  //验证密码是否正确
{
	if(wanbi)	 //只有当六位密码均输入完毕后方进行验证
	{
	if((new1==old1)&(new2==old2)&(new3==old3)&(new4==old4)&(new5==old5)&(new6==old6))
		allow=1;	//当输入的密码正确，会得到allowe置一
	}
}

void main()
{
	init();
	allow=1;
	P1=0x00;
	for(ii=0;ii<=10;ii++)// 降低音量
	{
	index = 0; // 重置索引
	while(index < 4) // 发送四个字节的数据
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
		if(confirm)	 //验证
		{
			confirm=0;
			ok=0; wei=0;
			yanzheng();
			if(allow)	 //验证完后，若allow为1，则开锁
			{
				P1=0x00;//
				index = 0; // 重置索引
				while(index < 6)
				{
						send(song1[index]); // 第1歌
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
					index = 0; // 重置索引
					while(index < 6)
					{
							send(song2[index]); // 第2歌
							index++;
					}
				}
				else if(Number_of_errors==2)
				{
					index = 0; // 重置索引
					while(index < 6)
					{
							send(song3[index]); // 第3歌
							index++;
					}
				}
				else if(Number_of_errors>=3)//bo fang yin yue
				{
					index = 0; // 重置索引
					while(index < 6)
					{
							send(song4[index]); // 第4歌
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
		if(genggai)	  //当S16更改密码键被按下，genggai会被置一
		{
			if(allow)	 //若已经把锁打开，才有更改密码的权限
			{
				while(!wanbi)	//当新的六位密码没有设定完，则一直在这里循环
				{
				 	keyscan();
					shumima();
					if(retry|close)	 //而当探测到重试键S18或者关闭密码锁键S19被按下时，则跳出
					{	wanbi=1;
						break;
					}
					display(a,b,c,d,e,f);
				}
			}
		}
		if(ok)	  //更改密码时，当所有六位新密码均被按下时，可以按下此键，结束密码更改
		{		  //其他时间按下此键无效
			ok=0; wei=0;
			genggai=0;
			old1=new1;old2=new2;old3=new3; //此时，旧的密码将被代替
			old4=new4;old5=new5;old6=new6;
			a=16;b=16;c=16;d=16;e=16;f=16;
		}
		if(retry)	//当重试按键S18被按下，retry会被置位
		{
		retry=0; wei=0;wanbi=0;
		a=16;b=16;c=16;d=16;e=16;f=16;
		new1=0;new2=0;new3=0;new4=0;new5=0;new6=0;		
		}
		if(close)  //当关闭密码锁按键被按下，close会被置位
		{
			close=0;genggai=0;//所有变量均被清零。
			wei=0;	wanbi=0;
			allow=0;
			P1=0xff;
			a=16;b=16;c=16;d=16;e=16;f=16;
			new1=0;new2=0;new3=0;new4=0;new5=0;new6=0;
		}
		display(a,b,c,d,e,f); //实时显示
	}
}
