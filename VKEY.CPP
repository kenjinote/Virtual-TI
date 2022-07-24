//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "vkey.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
Tvkeypad *vkeypad;
//---------------------------------------------------------------------------
__fastcall Tvkeypad::Tvkeypad(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    CanClose = false;
	Hide();
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::dummyinboxKeyPress(TObject *Sender, char &Key)
{
   	Key = 0;

}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button30Click(TObject *Sender)
{
	dummyinbox->SetFocus();	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::dummyinboxKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
    if (Key == 18) return;
	processkeydown(Key);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::dummyinboxKeyUp(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	if (Key == 18) return;
	processkeyup(Key);
}
//---------------------------------------------------------------------------

void Tvkeypad::processkeyup(WORD &Key)
{
    long i, j, k;

	if (shiftdown1 == 1)
    {
	    if (Key == 56 ) Key = 106;
	    else if (Key == 57 ) Key = 219;
	    else if (Key == 48 ) Key = 221;
        else if (Key == 187) Key = 107;
    }
    if (Key == 16) shiftdown1 = 0;

	if (*keymode == 0)
    {
	    j = 0; for (i=0; i<8; ++i)	{if ( (1<<i) & ~(*rowscan) ) ++j;}
		k = 0; for (i=8; i<16; ++i) {if ( (1<<i) & ~(*rowscan) ) ++k;}
		if (k > 1)
            // > 1 colum = error condition, reset keypad
        	*rowscan = 0xffff;
		else
        {
		   	if (j == 1)
		    	*rowscan = *rowscan | keytab[Key]; // normal
		    else
		    	*rowscan = *rowscan | (keytab[Key] & 0x00ff); // multi rows
        }
    }
    else
    	*rowscan = 0xffff;
}

void Tvkeypad::processkeydown(WORD &Key)
{
    if (Key == 16) shiftdown1 = 1;
	if (shiftdown1 == 1)
    {
	    if (Key == 56 ) Key = 106;
	    else if (Key == 57 ) Key = 219;
	    else if (Key == 48 ) Key = 221;
        else if (Key == 187) Key = 107;
    }
   	if (*keymode == 0)
    	*rowscan &= ~keytab[Key];
    else
    	*rowscan = ~keytab[Key];
}
void __fastcall Tvkeypad::Button21MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(85);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button21MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(85);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button1MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(112);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button2MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(113);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button3MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(114);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button4MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(115);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button5MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(116);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button6MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(17);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button14MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(27);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button22MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(45);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button7MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(192);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button23MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(46);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button8MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(117);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button16MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(118);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button33Click(TObject *Sender)
{
	processkeydown(120);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button40MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(220);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button9MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeydown(65);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button17MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(66);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button25MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(67);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button34MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(68);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button41MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(69);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button10MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(70);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button18MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(71);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button26MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(72);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button35MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(73);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button42MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(74);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button11MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(75);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button19MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(76);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button27MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(77);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button36MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(78);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button43MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(79);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button12MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(80);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button20MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(81);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button28MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(82);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button37MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(83);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button44MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(84);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button29MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(86);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button38MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(87);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button45MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(88);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button31MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(89);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button32MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(90);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button39MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(32);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button46MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(0x0d);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button13MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(187);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button1MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(112);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button2MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(113);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button3MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(114);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button4MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(115);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button5MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(116);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button50MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(38);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button47MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(37);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button49MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(40);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button48MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(39);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button6MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(17);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button14MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(27);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button22MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(45);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button7MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(192);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button23MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(46);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button8MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(117);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button16MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(118);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button24MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(119);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button33MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(120);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button40MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(220);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button9MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(65);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button17MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(66);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button25MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(67);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button34MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(68);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button41MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(69);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button10MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(70);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button18MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(71);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button26MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(72);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button35MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(73);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button42MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(74);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button11MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(75);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button19MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(76);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button27MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(77);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button36MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(78);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button43MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(79);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button12MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(80);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button20MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(81);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button28MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(82);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button37MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(83);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button44MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(84);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button13MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(187);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button29MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(86);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button38MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(87);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button45MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(88);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button31MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(89);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button32MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(90);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button39MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(32);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button46MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(0x0d);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button50MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(38);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button47MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(37);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button49MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(40);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button48MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(39);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button24MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(119);	
	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button33MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(120);	

}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button15MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	processkeydown(34);
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::Button15MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	processkeyup(34);	
}
//---------------------------------------------------------------------------
void __fastcall Tvkeypad::FormActivate(TObject *Sender)
{
	shiftdown1 = 0;	
}
//---------------------------------------------------------------------------
