/*	Copyright  (c)	Günter Woigk   2001-2005
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice, 
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice, 
	  this list of conditions and the following disclaimer in the documentation 
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	2002-05-20 kio	fixed NaN tests: IEEE ???
	2004-05-07 kio	updated FindItem() test ((does no longer find dot-separated sub items))
*/


#define SAFE 0
#define LOG  0

#include 	"var.h"


#ifdef INCLUDE_VAR_TEST_SUITE


#include	<math.h>
#ifndef NAN
	const double NAN = 0.0/0.0;
#endif


DEBUG_INIT_MSG("Var test suite <init-free>");



void TestVarClass()
{
	Log("\n+++ Testing class Var +++\n");

	{
		Log("\nAssumptions");
		{
			double n = NAN;

			TRAP(n==n);
			TRAP(!(n!=n));
			TRAP(n>n);
			TRAP(n>=n);
			TRAP(n<n);
			TRAP(n<=n);

			TRAP(1.0==n);
			TRAP(!(1.0!=n));
			TRAP(1.0>n);
			TRAP(1.0>=n);
			TRAP(1.0<n);
			TRAP(1.0<=n);

			TRAP(n==1.0);
			TRAP(!(n!=1.0));
			TRAP(n>1.0);
			TRAP(n>=1.0);
			TRAP(n<1.0);
			TRAP(n<=1.0);

		}
		Log(" ...ok\n");


		Log("\nVar Creators\n");

		Log("Var()");
		Var*v=new Var();
		TRAP(!v->IsNumber());
		TRAP(v->Value()!=0.0);
		TRAP(v->Name()!="");
		Log(" ...ok\n");

		Log("~Var()");
		delete v;
		Log(" ...ok\n");

		Log("Var(type)");
		v=new Var(isNumber);
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="");
		TRAP(v->Value()!=0.0);
		delete v;
		v=new Var(isText);
		TRAP(!v->IsText());
		TRAP(v->Name()!="");
		TRAP(v->Text()!="");
		delete v;
/*		v=new Var(isProc);
		TRAP(!v->IsProc());
		TRAP(v->Name()!="");
		TRAP(v->Proc()!="");
		delete v;
*/		v=new Var(isList);
		TRAP(!v->IsList());
		TRAP(v->Name()!="");
		TRAP(v->ListSize()!=0);
		delete v;
		Log(" ...ok\n");

		Log("Var(type,name)");
		v=new Var(isNumber,"Berta");
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="Berta");
		TRAP(v->Value()!=0.0);
		delete v;
		v=new Var(isText,"Caesar");
		TRAP(!v->IsText());
		TRAP(v->Name()!="Caesar");
		TRAP(v->Text()!="");
		delete v;
/*		v=new Var(isProc,"Dorothea");
		TRAP(!v->IsProc());
		TRAP(v->Name()!="Dorothea");
		TRAP(v->Proc()!="");
		delete v;
*/		v=new Var(isList,"Emily");
		TRAP(!v->IsList());
		TRAP(v->Name()!="Emily");
		TRAP(v->ListSize()!=0);
		delete v;
		Log(" ...ok\n");

		Log("Var(value)");
		v=new Var(47.11);
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="");
		TRAP(v->Value()!=47.11);
		delete v;
		v=new Var(0.0);
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="");
		TRAP(v->Value()!=0.0);
		delete v;
		v=new Var(4711);
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="");
		TRAP(v->Value()!=4711.0);
		delete v;
		Log(" ...ok\n");

		Log("Var(value,name)");
		v=new Var(47.11,"Anton");
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="Anton");
		TRAP(v->Value()!=47.11);
		delete v;
		v=new Var(0,"Berta");
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="Berta");
		TRAP(v->Value()!=0.0);
		delete v;
		v=new Var(4711,"Caesar");
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="Caesar");
		TRAP(v->Value()!=4711.0);
		delete v;
		Log(" ...ok\n");

		Log("Var(text)");
		v=new Var("oink oink!");
		TRAP(!v->IsText());
		TRAP(v->Name()!="");
		TRAP(v->Text()!="oink oink!");
		TRAP(v->Text().NotWritable());
		delete v;
		v=new Var("");
		TRAP(!v->IsText());
		TRAP(v->Name()!="");
		TRAP(v->Text()!="");
		TRAP(v->Text().NotWritable());
		delete v;
#if defined(_MACINTOSH)
		v=new Var("\pärger!");
		TRAP(!v->IsText());
		TRAP(v->Name()!="");
		TRAP(v->Text()!="ärger!");
		TRAP(v->Text().NotWritable());
		delete v;
#endif
		Log(" ...ok\n");

		Log("Var(text,name)");
		v=new Var("oink oink!","Pünktchen");
		TRAP(!v->IsText());
		TRAP(v->Name()!="Pünktchen");
		TRAP(v->Text()!="oink oink!");
		TRAP(v->Text().NotWritable());
	//	TRAP(v->Name().NotWritable());		Kopie des Textes im NameHandle-Modul
		delete v;
		v=new Var("","Schniedelwutz");
		TRAP(!v->IsText());
		TRAP(v->Name()!="Schniedelwutz");
		TRAP(v->Text()!="");
		TRAP(v->Text().NotWritable());
		delete v;
#if defined(_MACINTOSH)
		v=new Var("\pärger!","Omega");
		TRAP(!v->IsText());
		TRAP(v->Name()!="Omega");
		TRAP(v->Text()!="ärger!");
		TRAP(v->Text().NotWritable());
		delete v;
#endif
#if defined(_MACINTOSH)
		v=new Var("\pärger!","");
		TRAP(!v->IsText());
		TRAP(v->Name()!="");
		TRAP(v->Text()!="ärger!");
		TRAP(v->Text().NotWritable());
		delete v;
#endif
		Log(" ...ok\n");

		Log("Var(Var)");
		v=new Var( Var("berta","anton") );
		TRAP(!v->IsText());
		TRAP(v->Name()!="anton");
		TRAP(v->Text()!="berta");
		delete v;
		v=new Var( Var(47.11,"caesar") );
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="caesar");
		TRAP(v->Value()!=47.11);
		delete v;
		Log(" ...ok\n");

		Log("operator=()");
		v = new Var(0,"n");
		*v = Var("berta","anton");
		TRAP(!v->IsText());
		TRAP(v->Name()!="n");
		TRAP(v->Text()!="berta");
		*v = Var(47.11,"caesar");
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="n");
		TRAP(v->Value()!=47.11);
		*v = "alabaster";
		TRAP(!v->IsText());
		TRAP(v->Name()!="n");
		TRAP(v->Text()!="alabaster");
		*v = 47.11;
		TRAP(!v->IsNumber());
		TRAP(v->Name()!="n");
		TRAP(v->Value()!=47.11);
		delete v;
		Log(" ...ok\n");

		Log("All Var creator tests passed.\n");
	}



	{
		Log("\nMisc. Functions\n");
		Var a,b;
		a.SetName("anton");
		b.SetName("berta");

		Log("CopyData()");
		a="jdshgsj";
		b=47.11;
		a.CopyData(b);
		TRAP(!a.IsNumber());
		TRAP(!b.IsNumber());
		TRAP(a.Value()!=47.11);
		TRAP(b.Value()!=47.11);
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		a="Honolulu";
		b=47.11;
		b.CopyData(a);
		TRAP(!a.IsText());
		TRAP(!b.IsText());
		TRAP(a.Text()!="Honolulu");
		TRAP(b.Text()!="Honolulu");
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		Log(" ...ok\n");

		Log("MoveData()");
		a="jdshgsj";
		b=47.11;
		a.MoveData(b);
		TRAP(a.IsNoNumber());
		TRAP(b.IsNoNumber());
		TRAP(b.Value()!=0.0);
		TRAP(a.Value()!=47.11);
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		a="Honolulu";
		b=47.11;
		b.MoveData(a);
		TRAP(!a.IsNumber());
		TRAP(a.Value()!=0.0);
		TRAP(!b.IsText());
		TRAP(b.Text()!="Honolulu");
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		Log(" ...ok\n");

		Log("SwapData()");
		a="aubuquerqee";
		b=47.11;
		a.SwapData(b);
		TRAP(!a.IsNumber());
		TRAP(!b.IsText());
		TRAP(a.Value()!=47.11);
		TRAP(b.Text()!="aubuquerqee");
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		a="pleasentville";
		b=47.11;
		b.SwapData(a);
		TRAP(!a.IsNumber());
		TRAP(!b.IsText());
		TRAP(a.Value()!=47.11);
		TRAP(b.Text()!="pleasentville");
		TRAP(a.Name()!="anton");
		TRAP(b.Name()!="berta");
		Log(" ...ok\n");

		Log("SetType()");
		a="";
		a.SetType(isNumber);
		TRAP(!a.IsNumber());
		TRAP(a.Name()!="anton");
		TRAP(a.Value()!=0.0);
		a.SetType(isText);
		TRAP(!a.IsText());
		TRAP(a.Name()!="anton");
		TRAP(a.Text()!="");
/*		a.SetType(isProc);
		TRAP(!a.IsProc());
		TRAP(a.Name()!="anton");
		TRAP(a.ProcAddr()!=NULL);
		TRAP(a.ProcSize()!=0);
*/		a.SetType(isList);
		TRAP(!a.IsList());
		TRAP(a.Name()!="anton");
		TRAP(a.ListSize()!=0);
		a=0.0;
		Log(" ...ok\n");

		Log("Name()");
		a.SetName("xyz");
		b.SetName(a.Name());
		TRAP(a.Name()!=b.Name());
		TRAP(b.Name()!="xyz");
		TRAP(a.Name().Text()!=b.Name().Text());
		TRAP(!a.IsNamed());
		a.ClearName();
		TRAP(a.Name()!="");
		TRAP(b.Name()!="xyz");
		TRAP(a.IsNamed());
		TRAP(!b.IsNamed());
		a.SetName("anton");
		b.SetName("berta");
		Log(" ...ok\n");

		Log("Value()");
		a="";
		TRAP(!a.IsText());
		a=47.11;
		TRAP(!a.IsNumber());
		TRAP(a.Value()!=47.11);
		Log(" ...ok\n");

		Log("Text()");
		a="abcde";
		b=a;
		TRAP(!a.IsText());
		TRAP(!b.IsText());
		TRAP(b.Text()!="abcde");
		TRAP(a.TextAddr()!=a.Text().Text());
		TRAP(a.TextAddr()!=b.Text().Text());
		TRAP(a.TextLen()!=5);
		Log(" ...ok\n");

		Log("Proc()");
		// •••
		Log(" ...ok\n");

#if 0
		Log("ListVar()");
		a.SetList();
		a[0]=1;
		a[1]="Wörterbücher";
		a[2]="\"Ligaturen\"";
		a[1]=a;
		LogNL();Log("Soll: { 1 { 1 \"Wörterbücher\" \"\\\"Ligaturen\\\"\" } \"\\\"Ligaturen\\\"\" }");
		LogNL();Log("Ist:  ");a.LogVar();
		LogNL();Log(" ...ok ?\n");
#endif

		Log("MaxVar() et.al.");
		a=3;b=5;
		TRAP(&a.MaxVar(b)!=&b);
		TRAP(&b.MaxVar(a)!=&b);
		TRAP(&a.MinVar(b)!=&a);
		TRAP(&b.MinVar(a)!=&a);
		a.SetList();a[0]=3;a[1]=5;a[2]=1;a[3]=5;a[4]=1;
		TRAP(&a.MaxVar()!=&a[1]);
		TRAP(&a.MinVar()!=&a[2]);
		Log(" ...ok\n");

		Log("Misc. function tests passed.\n");
	}



	{
		Log("\nList-related Functions\n");
		Var a(0,"a"),b(0,"b");

		Log("SetType(isList)");
		a.SetType(isList);
		TRAP(!a.IsList());
		TRAP(a.List().array!=NULL);
		TRAP(a.List().size!=0);
		TRAP(a.List().used!=0);
		TRAP(a.ListSize()!=0);
		Log(" ...ok\n");

		Log("operator[]()");
		a[5]="Gummibärchen";
		TRAP(a.List().array==NULL);
		TRAP(a.List().size<a.List().used);
		TRAP(a.List().used!=6);
		TRAP(a.ListSize()!=6);
		a="x";
		TRAP(a.Text()!="x");
		a.SetType(isList);
		a.SetType(isList);
		TRAP(!a.IsList());
		a[4]="Anaconda";
		a[4].SetName("Boa Constrictor");
		TRAP(a.Parent()!=NULL);
		TRAP(a[4].Parent()!=&a);
		TRAP(a[0]!=0.0);
		TRAP(a[5]!=0.0);
		TRAP(a.ListSize()!=6);
		TRAP(a[4].Text()!="Anaconda");
		TRAP(Error());
		Log(" ...ok\n");

		Log("Contains()");
		TRAP(a[3].Contains(&a));
		TRAP(a.Contains(b));
		TRAP(b.Contains(a));
		TRAP(!b.Contains(b));
		TRAP(!a.Contains(a));
		TRAP(!a.Contains(a[3]));
		a[99]=47.11;
		TRAP(a[4].Text()!="Anaconda");
		TRAP(a[99].Value()!=47.11);
		TRAP(Error());
		Log(" ...ok\n");

		Log("FindItem()");
		a[4]=0.0;
		TRAP(a[4].Name()!="Boa Constrictor");
		TRAP(!a.FindItem("Boa Constrictor"));
		TRAP(a.FindItem("Boa Constrictor")->Parent() != &a);
		TRAP(a.FindItem("Boa Constrictor")->Index() != 4);
		TRAP(!a.FindItem(""));
		TRAP(a.FindItem("")->Index() != a.ListSize()-1);
		a[66].SetName("aber");
		a[77].SetName("aber");
		a[88].SetName("aber ");
		a[77].SetType(isList);
		a[77][4].SetName("hallo!");	// <--
		a[77][4] = 4711;
		a[77][5].SetName("hallo!");
		a[77][9].SetName("hallo");
		a[77][17];
		a[77][5] = "ey boah ey!";
		TRAP(!a.Contains(a[77][3]));
		Var* p;
		TRAP( (p = a.FindItem("aber")) == NULL );
		TRAP( (p = p->FindItem("hallo!")) == NULL );
		TRAP( p->GetType()!=isText );
		TRAP( p->Text()!="ey boah ey!" );
		a[77].ResizeList(5);
		TRAP(a[77].List().size!=5);
		TRAP(a[77].List().used!=5);
		TRAP( (p = a.FindItem("aber")) == NULL );
		TRAP( (p = p->FindItem("hallo!")) == NULL );
		TRAP( p->GetType()!=isNumber );
		TRAP( p->Value()!=4711 );
		TRAP(Error());
		Log(" ...ok"); LogNL();

		Log("Resize()");
		a.ResizeList(30);
		TRAP(!a.IsList());
		TRAP(a.List().array==NULL);
		TRAP(a.List().used!=30);
		TRAP(a.List().size!=30);
		a.ResizeList(40);
		TRAP(!a.IsList());
		TRAP(a.List().array==NULL);
		TRAP(a.List().used!=40);
		TRAP(a.List().size<40);
		a.ResizeList(0);
		TRAP(!a.IsList());
		TRAP(a.List().array!=NULL);
		TRAP(a.List().used!=0);
		TRAP(a.List().size!=0);
		TRAP(Error());
		Log(" ...ok\n");

		Log("InsertItems()");
		a[0]=0.0;
		a[1]=1;
		a[2]=2;
		a[3]=3;
		a.InsertItems(2,3);
		TRAP(a.ListSize()!=7);
		TRAP(a[0]!=0);
		TRAP(a[1]!=1);
		TRAP(a[2+3]!=2);
		TRAP(a[3+3]!=3);
		TRAP(a[2]!=0.0);
		TRAP(a[3]!=0.0);
		TRAP(a[4]!=0.0);
		a.InsertItem(77);
		TRAP(a.ListSize()!=7);
		a.InsertItem(0);
		TRAP(a.ListSize()!=8);
		TRAP(a[3+3+1]!=3);
		TRAP(a[0  +1]!=0);
		TRAP(Error());
		Log(" ...ok\n");

		Log("DeleteItems()");
		a.DeleteItem(0);
		TRAP(a.ListSize()!=7);
		TRAP(a[3+3]!=3);
		TRAP(a[0  ]!=0);
		a.DeleteItem(77);
		TRAP(a.ListSize()!=7);
		a.DeleteItems(2,3);
		TRAP(a.ListSize()!=4);
		TRAP(a[0]!=0);
		TRAP(a[1]!=1);
		TRAP(a[2]!=2);
		TRAP(a[3]!=3);
		TRAP(Error());
		Log(" ...ok\n");

		Log("AppendItem()");
		b=a;
		a.AppendItem(new Var("haha"));
		b[b.ListSize()]="haha";
		TRAP(a!=b);
		Log(" ...ok\n");

		Log("AppendList()");
		b.ResizeList(2);
		a.AppendListItems(b);
		TRAP(a.ListSize()!=7);
		TRAP(a[0]!=0);
		TRAP(a[1]!=1);
		TRAP(a[2]!=2);
		TRAP(a[3]!=3);
		TRAP(a[4]!="haha");
		TRAP(a[5]!=0);
		TRAP(a[6]!=1);
		TRAP(Error());
		Log(" ...ok\n");

		Log("Sort()");
		b=a;
		a.Sort();
		TRAP(a.ListSize()!=7);
		TRAP(a[0]!=0);
		TRAP(a[1]!=0);
		TRAP(a[2]!=1);
		TRAP(a[3]!=1);
		TRAP(a[4]!=2);
		TRAP(a[5]!=3);
		TRAP(a[6]!="haha");
		b.RSort();
		TRAP(b.ListSize()!=7);
		TRAP(b[0]!="haha");
		TRAP(b[1]!=3);
		TRAP(b[2]!=2);
		TRAP(b[3]!=1);
		TRAP(b[4]!=1);
		TRAP(b[5]!=0);
		TRAP(b[6]!=0);
		TRAP(Error());
		Log(" ...ok\n");

		Log("operator=()");
		a.SetList();a[0]=1;a[1]=2;a[2]=3;a[3]=4;a[4]=5;
		Log("."); b=a;
		Log("."); a[3]=a;
		Log("."); TRAP(a.ListSize()!=5);
		Log("."); TRAP(a[0]!=1);
		Log("."); TRAP(a[1]!=2);
		Log("."); TRAP(a[2]!=3);
		Log("."); TRAP(a[3]!=b);
		Log("."); TRAP(a[4]!=5);
		Log("."); a=a[3];
		Log("."); TRAP(a!=b);
		TRAP(Error());
		Log(" ...ok\n");

		Log("List-related functions tests passed.\n");
	}



	{
		Log("\nOperators\n");
		Var a,b;
		a.SetList();
		a[0]=0.0;a[1]=1;a[2]=2;

		Log("operator==()");
		TRAP(  Var(1)==Var(2));
		TRAP(!(Var(1)!=Var(2)));
		TRAP(!(Var(1)==Var(1)));
		TRAP(  Var("1")==Var("2"));
		TRAP(!(Var("1")!=Var("2")));
		TRAP(!(Var("1")==Var("1")));
		TRAP(Var()!=0);
		TRAP(Var(1)==Var("1"));
		b=a; TRAP(a!=b);
		b[3]=3; TRAP(a==b);
		TRAP(a==Var(""));
		TRAP(NoError()); ClearError();
		Log(" ...ok\n");
	}

	{
		Log("operator>");
		Var a,b;
		a.SetList();
		a[0]=0.0;a[1]=1;a[2]=2;

		TRAP(  Var(1)>Var(2));
		TRAP(!(Var(1)<=Var(2)));
		TRAP(!(Var(1)<=Var(1)));
		TRAP(  Var("1")>Var("2"));
		TRAP(!(Var("1")<=Var("2")));
		TRAP(!(Var("1")<=Var("1")));
#if !defined(_GCC)
		TRAP(Var()>0);
#else
		Var v = Var();
		TRAP(v>0);
#endif
		TRAP(Error());
		TRAP(Var(1)>Var("1"));
		TRAP(NoError()); ClearError();
		b=a;
		TRAP(a>b);
		TRAP(a<b);
		TRAP(a!=b);
		b[3]=3; TRAP(a>b);
		b[2]=3; TRAP(a>b);
		b[2]=0.0; TRAP(a<=b);
		TRAP(Error());
		TRAP(a<=Var(""));
		TRAP(NoError()); ClearError();
		Log(" ...ok\n");
	}

	{
		Log("operator<");
		Var a,b;
		a.SetList();
		a[0]=0.0;a[1]=1;a[2]=2;

		TRAP(  Var(2)<Var(1));
		TRAP(!(Var(2)>=Var(1)));
		TRAP(!(Var(1)>=Var(1)));
		TRAP(  Var("2")<Var("1"));
		TRAP(!(Var("2")>=Var("1")));
		TRAP(!(Var("1")>=Var("1")));
		TRAP(Var(0.0)<Var());
		TRAP(Error());
		TRAP(Var("1")<Var(1));
		TRAP(NoError()); ClearError();
		b=a; TRAP(a<b);
		b[3]=3; TRAP(b<a);
		b[2]=3; TRAP(b<a);
		b[2]=0.0; TRAP(b>=a);
		TRAP(Error());
		TRAP(Var("")>=a);
		TRAP(NoError()); ClearError();
		Log(" ...ok\n");

		Log("operator+=");
		a=10;a+=37.11;TRAP(a!=47.11);
		a="An";a+="ton";TRAP(a!=Var("Anton"));
		a="abc"; a+=a;
		TRAP(Error()!=notanumber); ClearError();
		a.SetList();
		a[0]=3;
		a[1]="abc";
		a[2]=a;				// a = { 3 "abc" { 3 "abc" 0 } }
		TRAP(!a[2].IsList());
		TRAP(a[2][2]!=0.0);
		a += a;				// a = { 6 NaN { 6 NaN 0.0 } }
		TRAP(Error()!=notanumber); ClearError();
		TRAP(a[0]!=6);
	//	TRAP(a[1]!=0.0/0.0);
		TRAP(a[1]==a[1]);		// IEEE: NaN != NaN  ???
		TRAP(!a[2].IsList());
		TRAP(a[2][0]!=a[0]);
	//	TRAP(a[2][1]!=a[1]);
		TRAP(a[2][1]==a[2][1]);		// IEEE: NaN != NaN  ???
		TRAP(!a[2][2].IsNumber());
	//	TRAP(a[2][2]!=0.0/0.0);
		TRAP(a[2][2]!=0.0);		// 2003-08-10 kio neu: operator+ casted argumente nach number => * := 0.0
		Log(" ...ok\n");

		Log("operator-=");
		a[0]=5;a[1]=7;a[2]=9;a[2]=a;				// a = { 5 7 { 5 7 9 } }
		a-=1;
		TRAP(a[0]!=4);
		TRAP(a[1]!=6);
		TRAP(a[2][0]!=4);
		TRAP(a[2][2]!=8);
		Log(" ...ok\n");

		Log("operator*=");
		b=a;
		a*=5;
		TRAP(a[0]!=4*5);
		TRAP(a[1]!=6*5);
		TRAP(a[2][0]!=4*5);
		TRAP(a[2][2]!=8*5);
		Log(" ...ok\n");

		Log("operator/=");
		a/=5;
		TRAP(a!=b);
		Log(" ...ok"); LogNL();

		Log("%s","operator%=");
		b=a;
		a%=5;
		TRAP(a[0]!=4%5);
		TRAP(a[1]!=6%5);
		TRAP(a[2][0]!=4%5);
		TRAP(a[2][2]!=8%5);
		Log(" ...ok\n");

		Log("%s","operator+ .. operator%");
		TRAP(Var(4)+5!=4+5);
		TRAP(Var(4)+Var(5)!=4+5);
//		TRAP(Var("a")+4!="a4");
		TRAP(Var("a")+"b"!="ab");
		a.SetList();a[0]=4;  a[1]=6;
		b.SetList();b[0]=4+3;b[1]=6+3;
		TRAP(a+3!=b);
		TRAP(Var(4)-5!=4-5);
		TRAP(Var(4)-Var(5)!=4-5);
		a.SetList();a[0]=4;  a[1]=6;
		b.SetList();b[0]=4-3;b[1]=6-3;
		TRAP(a-3!=b);
		TRAP(Var(4)*5!=4*5);
		TRAP(Var(4)*Var(5)!=4*5);
//		TRAP(Var("a")*4!="aaaa");
		a.SetList();a[0]=4;  a[1]=6;
		b.SetList();b[0]=4*3;b[1]=6*3;
		TRAP(a*3!=b);
		TRAP(Var(40)/5!=40/5);
		TRAP(Var(40)/Var(5)!=40/5);
		a.SetList();a[0]=40;    a[1]=60;
		b.SetList();b[0]=40.0/3;b[1]=60.0/3;
		TRAP(a/3!=b);
		TRAP(Var(42)%5!=42%5);
		TRAP(Var(41)%Var(5)!=41%5);
		a.SetList();a[0]=43;  a[1]=64;
		b.SetList();b[0]=43%3;b[1]=64%3;
		TRAP(a%3!=b);
		Log(" ...ok\n");

		Log("Operator tests passed.\n");
	}
}


#endif

