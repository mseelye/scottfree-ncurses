/*
 *  ScottFree-ncurses, a reworking of ScottFree Revision 1.14b to run in ncurses
 *  (C) 2020 - Mark Seelye / mseelye@yahoo.com
 *
 *  Primarly updated to run with ncurses windows. Some small tweaks too.
 *  I made "mseelye" comments near most my changes to 1.14b. There
 *  are some that are not related to ncurses, but the majority are.
 *  There are other comments in this source, from the SCOTT.C source,
 *  they are either labeled with initials, or not labled at all. Those
 *  are not my comments.
 *  Did a fix to the RandomPercent fuction which was flawed a bit. (Thanks Per).
 *
 *  Build with:
 *    make
 *    The Makefile, which is essentially:
 *    gcc -o scottfree-ncurses.exe scottfree-ncurses.c -lncurses
 *
 *  Run with:
 *    ./out/scottfree-ncurses games/ghostking.dat
 *
 *  If terminal is wonky after running try:
 *    stty sane
 *
 *  Must have ncurses installed, and terminal should support color.
 *  I've tested this with Windows MSys(xterm), Mac, and Ubuntu.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Original copyright and license for ScottFree Revision 1.14b follows:
 */
/*
 *	ScottFree Revision 1.14b
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *
 *	You must have an ANSI C compiler to build this program.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

// mseelye - added unistd for strcasecmp, no stricmp or the like
#include <unistd.h>
#include <curses.h>

#include "scottfree.h"

#define TRUE 1
#define FALSE 0

Header GameHeader;
Tail GameTail;
Item *Items;
Room *Rooms;
char **Verbs;
char **Nouns;
char **Messages;
Action *Actions;
int LightRefill;
char NounText[16];
int Counters[16];    /* Range unknown */
int CurrentCounter;
int SavedRoom;
int RoomSaved[16];    /* Range unknown */
int DisplayUp;        /* Curses up */
void *Top,*Bottom;
int Redraw;        /* Update item window */
int Options;        /* Option flags set */
int Width;        /* Terminal width */
int TopHeight;        /* Height of top window */
int BottomHeight;    /* Height of bottom window */

#define TRS80_LINE    "\n<------------------------------------------------------------>\n"

#define TOPCOL    COLOR_MAGENTA // mseelye, Should be BROWN, oh well
#define BOTCOL    COLOR_BLUE

#define MyLoc    (GameHeader.PlayerRoom)

long BitFlags=0;    /* Might be >32 flags - I haven't seen >32 yet */

// mseelye - these are for the ncurses windows
WINDOW *top_win; 
WINDOW *bot_win;
void setupWindows();

// mseelye - Allow program to exit with 0 when done or 1 when error.
void Close(int code)
{
    endwin();
    exit(code);
}

// mseelye - modified this to output message twice in case ncurses issue
//           added getch(); for pause before exiting
void Fatal(char *x)
{
    printw("%s\n\r",x);
    printf("%s\n\r",x);
    refresh();
    getch();
    Close(1);
}

void ClearScreen(void)
{
    // mseelye - replaced this with clears to the ncurses windows
    wclear(top_win);
    wclear(bot_win);
}

void *MemAlloc(int size)
{
    void *t=(void *)malloc(size);
    if(t==NULL)
        Fatal("Out of memory");
    return(t);
}

// 1.14b steps random %: 1-4=%4 5-8:%7  9-12:%10 etc
// Updated to be more like ScottFree Java version 1.18.
int RandomPercent(uint8_t percent) {
    int val = rand() % 100;
    // cc65's srand surpresses sign bit, checking anyway
    if(val < 0) {
        val = -val;
    }
    if (val >= (int)percent) {
        return(0);
    }
    return(1);
}

int CountCarried()
{
    int ct=0;
    int n=0;
    while(ct<=GameHeader.NumItems)
    {
        if(Items[ct].Location==CARRIED)
            n++;
        ct++;
    }
    return(n);
}

char *MapSynonym(char *word)
{
    int n=1;
    char *tp;
    static char lastword[16];    /* Last non synonym */
    while(n<=GameHeader.NumWords)
    {
        tp=Nouns[n];
        if(*tp=='*')
            tp++;
        else
            strcpy(lastword,tp);
        if(strncasecmp(word,tp,GameHeader.WordLength)==0)
            return(lastword);
        n++;
    }
    return(NULL);
}

int MatchUpItem(char *text, int loc)
{
    char *word=MapSynonym(text);
    int ct=0;
    
    if(word==NULL)
        word=text;
    
    while(ct<=GameHeader.NumItems)
    {
        if(Items[ct].AutoGet && Items[ct].Location==loc &&
            strncasecmp(Items[ct].AutoGet,word,GameHeader.WordLength)==0)
            return(ct);
        ct++;
    }
    return(-1);
}

char *ReadString(FILE *f)
{
    char tmp[1024];
    char *t;
    int c,nc;
    int ct=0;
    do
    {
        c=fgetc(f);
    }
    while(c!=EOF && isspace(c));
    if(c!='"')
    {
        Fatal("Initial quote expected");
    }
    do
    {
        c=fgetc(f);
        if(c==EOF)
            Fatal("EOF in string");
        if(c=='"')
        {
            nc=fgetc(f);
            if(nc!='"')
            {
                ungetc(nc,f);
                break;
            }
        }
        if(c==0x60) 
            c='"'; /* pdd */
        if(c=='\n')
        {
            tmp[ct++]=c;
            c='\r';    /* 1.12a PC - pdd */
        }
        tmp[ct++]=c;
    }
    while(1);
    tmp[ct]=0;
    t=MemAlloc(ct+1);
    memcpy(t,tmp,ct+1);
    return(t);
}

void LoadDatabase(FILE *f, int loud)
{
    int ni,na,nw,nr,mc,pr,tr,wl,lt,mn,trm;
    int ct;
    short lo;
    Action *ap;
    Room *rp;
    Item *ip;
/* Load the header */
    // mseelye - changed this to load that first magic number. compiler warnings.
    if(fscanf(f,"%d %d %d %d %d %d %d %d %d %d %d %d",
        &ct,&ni,&na,&nw,&nr,&mc,&pr,&tr,&wl,&lt,&mn,&trm)<12)
        Fatal("Invalid database(bad header)");
    GameHeader.NumItems=ni;
    Items=(Item *)MemAlloc(sizeof(Item)*(ni+1));
    GameHeader.NumActions=na;
    Actions=(Action *)MemAlloc(sizeof(Action)*(na+1));
    GameHeader.NumWords=nw;
    GameHeader.WordLength=wl;
    Verbs=(char **)MemAlloc(sizeof(char *)*(nw+1));
    Nouns=(char **)MemAlloc(sizeof(char *)*(nw+1));
    GameHeader.NumRooms=nr;
    Rooms=(Room *)MemAlloc(sizeof(Room)*(nr+1));
    GameHeader.MaxCarry=mc;
    GameHeader.PlayerRoom=pr;
    GameHeader.Treasures=tr;
    GameHeader.LightTime=lt;
    LightRefill=lt;
    GameHeader.NumMessages=mn;
    Messages=(char **)MemAlloc(sizeof(char *)*(mn+1));
    GameHeader.TreasureRoom=trm;

/* Load the actions */

    ct=0;
    ap=Actions;
    if(loud)
        printf("Reading %d actions.\n\r",na);
    while(ct<na+1)
    {
        if(fscanf(f,"%hd %hd %hd %hd %hd %hd %hd %hd",
            &ap->Vocab,
            &ap->Condition[0],
            &ap->Condition[1],
            &ap->Condition[2],
            &ap->Condition[3],
            &ap->Condition[4],
            &ap->Action[0],
            &ap->Action[1])!=8)
        {
            printf("Bad action line (%d)\n\r",ct);
            exit(1);
        }
        ap++;
        ct++;
    }
    ct=0;
    if(loud)
        printf("Reading %d word pairs.\n\r",nw);
    while(ct<nw+1)
    {
        Verbs[ct]=ReadString(f);
        Nouns[ct]=ReadString(f);
        ct++;
    }
    ct=0;
    rp=Rooms;
    if(loud)
        printf("Reading %d rooms.\n\r",nr);
    while(ct<nr+1)
    {
        fscanf(f,"%hd %hd %hd %hd %hd %hd",
            &rp->Exits[0],&rp->Exits[1],&rp->Exits[2],
            &rp->Exits[3],&rp->Exits[4],&rp->Exits[5]);
        rp->Text=ReadString(f);
        ct++;
        rp++;
    }
    ct=0;
    if(loud)
        printf("Reading %d messages.\n\r",mn);
    while(ct<mn+1)
    {
        Messages[ct]=ReadString(f);
        ct++;
    }
    ct=0;
    if(loud)
        printf("Reading %d items.\n\r",ni);
    ip=Items;
    while(ct<ni+1)
    {
        ip->Text=ReadString(f);
        ip->AutoGet=strchr(ip->Text,'/');
        /* Some games use // to mean no auto get/drop word! */
        if(ip->AutoGet && strcmp(ip->AutoGet,"//") && strcmp(ip->AutoGet,"/*"))
        {
            char *t;
            *ip->AutoGet++=0;
            t=strchr(ip->AutoGet,'/');
            if(t!=NULL)
                *t=0;
        }
        fscanf(f,"%hd",&lo);
        ip->Location=(unsigned char)lo;
        ip->InitialLoc=ip->Location;
        ip++;
        ct++;
    }
    ct=0;
    /* Discard Comment Strings */
    while(ct<na+1)
    {
        free(ReadString(f));
        ct++;
    }
    fscanf(f,"%d",&ct);
    if(loud)
        printf("Version %d.%02d of Adventure ",
        ct/100,ct%100);
    fscanf(f,"%d",&ct);
    if(loud)
        printf("%d.\n\rLoad Complete.\n\r\n\r",ct);
}

int OutputPos=0;

void OutReset()
{
    OutputPos=0;
    // mseelye - ncurses changes here
    wmove(bot_win, BottomHeight, 1);
    wclrtoeol(bot_win);
    wrefresh(bot_win);
}

// mseelye - several ncurses changes in there, anything to do with output
void OutBuf(char *buffer)
{
    char word[80];
    int wp;

    while(*buffer)
    {
        if(OutputPos==0)
        {
            while(*buffer && isspace((unsigned char)*buffer))
            {
                if(*buffer=='\n')
                {
                    wmove(bot_win, BottomHeight, 1);
                    wprintw(bot_win, "\n\r");
                    wclrtoeol(bot_win);
                    OutputPos=0;
                }
                buffer++;
            }
        }
        if(*buffer==0)
            return;
        wp=0;
        while(*buffer && !isspace((unsigned char)*buffer))
        {
            word[wp++]=*buffer++;
        }
        word[wp]=0;
/*        fprintf(stderr,"Word '%s' at %d\n",word,OutputPos);*/
        if(OutputPos+strlen(word)>(Width-2))
        {
            wmove(bot_win, BottomHeight, 1);
            wprintw(bot_win, "\n\r");
            wclrtoeol(bot_win);
            OutputPos=0;
        }
        else
            wmove(bot_win, BottomHeight, OutputPos+1);
        wprintw(bot_win, "%s", word);
        OutputPos+=strlen(word);
        if(*buffer==0)
            return;
        if(*buffer=='\n')
        {
            wmove(bot_win, BottomHeight, 1);
            wprintw(bot_win,"\n\r");
            wclrtoeol(bot_win);
            OutputPos=0;
        }
        else
        {
            OutputPos++;
            if(OutputPos<(Width-1))
                wprintw(bot_win," ");
        }
        buffer++;
    }
    
    wrefresh(bot_win); // mseelye - ncurses needs refresh here
}

void Output(char *a)
{
    char block[512];
    strcpy(block,a);
    OutBuf(block);
}

void OutputNumber(int a)
{
    char buf[16];
    sprintf(buf,"%d",a);
    OutBuf(buf);
}

// mseelye - Look has ncurses changes to any output
void Look()
{
    static char *ExitNames[6]=
    {
        "North","South","East","West","Up","Down"
    };
    Room *r;
    int ct,f;
    int pos;

    wclear(top_win);

    if((BitFlags&(1L<<DARKBIT)) && Items[LIGHT_SOURCE].Location!= CARRIED
            && Items[LIGHT_SOURCE].Location!= MyLoc)
    {
        if(Options&YOUARE)
            wprintw(top_win,"You can't see. It is too dark!\n\r");
        else
            wprintw(top_win,"I can't see. It is too dark!\n\r");
        if (Options & TRS80_STYLE)
            wprintw(top_win,TRS80_LINE);
        wrefresh(top_win);
        return;
    }
    r=&Rooms[MyLoc];
    if(*r->Text=='*')
        wprintw(top_win,"%s\n\r",r->Text+1);
    else
    {
        if(Options&YOUARE)
            wprintw(top_win,"You are %s\n\r",r->Text);
        else
            wprintw(top_win,"I'm in a %s\n\r",r->Text);
    }
    ct=0;
    f=0;
    wprintw(top_win,"\n\rObvious exits: ");
    while(ct<6)
    {
        if(r->Exits[ct]!=0)
        {
            if(f==0)
                f=1;
            else
                wprintw(top_win,", ");
            wprintw(top_win,"%s",ExitNames[ct]);
        }
        ct++;
    }
    if(f==0)
        wprintw(top_win,"none");
    wprintw(top_win,".\n\r");
    ct=0;
    f=0;
    pos=0;
    while(ct<=GameHeader.NumItems)
    {
        if(Items[ct].Location==MyLoc)
        {
            if(f==0)
            {
                if(Options&YOUARE)
                    wprintw(top_win,"\n\rYou can also see: ");
                else
                    wprintw(top_win,"\n\rI can also see: ");
                pos=16;
                f++;
            }
            else if (!(Options & TRS80_STYLE))
            {
                wprintw(top_win," - ");
                pos+=3;
            }
            if(pos+strlen(Items[ct].Text)>(Width-10))
            {
                pos=0;
                wprintw(top_win,"\n\r");
            }
            wprintw(top_win,"%s",Items[ct].Text);
            pos += strlen(Items[ct].Text);
            if (Options & TRS80_STYLE)
            {
                wprintw(top_win,". ");
                pos+=2;
            }
        }
        ct++;
    }
    wprintw(top_win,"\n\r");
    if (Options & TRS80_STYLE)
        printw(TRS80_LINE);

    wrefresh(top_win); // mseelye - ncurses needs refresh
}

int WhichWord(char *word, char **list)
{
    int n=1;
    int ne=1;
    char *tp;
    while(ne<=GameHeader.NumWords)
    {
        tp=list[ne];
        if(*tp=='*')
            tp++;
        else
            n=ne;
        if(strncasecmp(word,tp,GameHeader.WordLength)==0)
            return(n);
        ne++;
    }
    return(-1);
}

// mseelye - updated this for ncurses
void LineInput(char *buf)
{
    int pos=0;
    int ch;

    wmove(bot_win, BottomHeight, 1+OutputPos);

    while(1)
    {
        ch=wgetch(bot_win);
        switch(ch)
        {
            case 10:;
            case 13:;
                buf[pos]=0;
                wprintw(bot_win,"\n\r");
                wrefresh(bot_win);
                return;
            case 8:;
            case 127:;
                if(pos>0)
                {
                    wprintw(bot_win,"\010");
                    wclrtoeol(bot_win); // mseelye - ncurses needs to clear to end of line
                    wrefresh(bot_win); // mseelye - ncurses needs refresh
                    pos--;
                }
                break;
            case KEY_RESIZE: 
                // mseelye - ncurses Resise event is a key sent to wgetch
                setupWindows();
                break;
            default:
                if(ch>=' '&&ch<=126)
                {
                    buf[pos++]=ch;
                    wprintw(bot_win,"%c",(char)ch);
                }
                break;
        }
    }
}

void GetInput(vb,no)
int *vb,*no;
{
    char buf[256];
    char verb[10],noun[10];
    int vc,nc;
    int num;
    do
    {
        do
        {
            Output("\nTell me what to do ? ");
            LineInput(buf);
            OutReset();
            num=sscanf(buf,"%9s %9s",verb,noun);
        }
        while(num==0||*buf=='\n');
        if(num==1)
            *noun=0;
        if(*noun==0 && strlen(verb)==1)
        {
            switch( isupper((unsigned char)*verb)?tolower(*verb):*verb)
            {
                case 'n':strcpy(verb,"NORTH");break;
                case 'e':strcpy(verb,"EAST");break;
                case 's':strcpy(verb,"SOUTH");break;
                case 'w':strcpy(verb,"WEST");break;
                case 'u':strcpy(verb,"UP");break;
                case 'd':strcpy(verb,"DOWN");break;
                /* Brian Howarth interpreter also supports this */
                case 'i':strcpy(verb,"INVENTORY");break;
            }
        }
        nc=WhichWord(verb,Nouns);
        /* The Scott Adams system has a hack to avoid typing 'go' */
        if(nc>=1 && nc <=6)
        {
            vc=1;
        }
        else
        {
            vc=WhichWord(verb,Verbs);
            nc=WhichWord(noun,Nouns);
        }
        *vb=vc;
        *no=nc;
        if(vc==-1)
        {
            Output("\"");
            Output(verb);
            Output("\" is a word I don't know...sorry!\n");
        }
    }
    while(vc==-1);
    strcpy(NounText,noun);    /* Needed by GET/DROP hack */
}

void SaveGame()
{
    char buf[256];
    int ct;
    FILE *f;
    Output("Filename: ");
    LineInput(buf);
    Output("\n");
    f=fopen(buf,"w");
    if(f==NULL)
    {
        Output("Unable to create save file.\n");
        return;
    }
    for(ct=0;ct<16;ct++)
    {
        fprintf(f,"%d %d\n",Counters[ct],RoomSaved[ct]);
    }
    fprintf(f,"%ld %d %hd %d %d %hd\n",BitFlags, (BitFlags&(1L<<DARKBIT))?1:0,
        MyLoc,CurrentCounter,SavedRoom,GameHeader.LightTime);
    for(ct=0;ct<=GameHeader.NumItems;ct++)
        fprintf(f,"%hd\n",(short)Items[ct].Location);
    fclose(f);
    Output("Saved.\n");
}

void LoadGame(char *name)
{
    FILE *f=fopen(name,"r");
    int ct=0;
    short lo;
    //short DarkFlag;
    int DarkFlag; // mseelye - fscanf complains when reading mix of int and short
    if(f==NULL)
    {
        Output("Unable to restore game.");
        return;
    }
    for(ct=0;ct<16;ct++)
    {
        fscanf(f,"%d %d\n",&Counters[ct],&RoomSaved[ct]);
    }
    fscanf(f,"%ld %d %hd %d %d %hd\n",
        &BitFlags,&DarkFlag,&MyLoc,&CurrentCounter,&SavedRoom,
        &GameHeader.LightTime);
    /* Backward compatibility */
    if(DarkFlag)
        BitFlags|=(1L<<15);
    for(ct=0;ct<=GameHeader.NumItems;ct++)
    {
        fscanf(f,"%hd\n",&lo);
        Items[ct].Location=(unsigned char)lo;
    }
    fclose(f);
}

int PerformLine(int ct)
{
    int continuation=0;
    int param[5],pptr=0;
    int act[4];
    int cc=0;
    while(cc<5)
    {
        int cv,dv;
        cv=Actions[ct].Condition[cc];
        dv=cv/20;
        cv%=20;
        switch(cv)
        {
            case 0:
                param[pptr++]=dv;
                break;
            case 1:
                if(Items[dv].Location!=CARRIED)
                    return(0);
                break;
            case 2:
                if(Items[dv].Location!=MyLoc)
                    return(0);
                break;
            case 3:
                if(Items[dv].Location!=CARRIED&&
                    Items[dv].Location!=MyLoc)
                    return(0);
                break;
            case 4:
                if(MyLoc!=dv)
                    return(0);
                break;
            case 5:
                if(Items[dv].Location==MyLoc)
                    return(0);
                break;
            case 6:
                if(Items[dv].Location==CARRIED)
                    return(0);
                break;
            case 7:
                if(MyLoc==dv)
                    return(0);
                break;
            case 8:
                if((BitFlags&(1L<<dv))==0)
                    return(0);
                break;
            case 9:
                if(BitFlags&(1L<<dv))
                    return(0);
                break;
            case 10:
                if(CountCarried()==0)
                    return(0);
                break;
            case 11:
                if(CountCarried())
                    return(0);
                break;
            case 12:
                if(Items[dv].Location==CARRIED||Items[dv].Location==MyLoc)
                    return(0);
                break;
            case 13:
                if(Items[dv].Location==0)
                    return(0);
                break;
            case 14:
                if(Items[dv].Location)
                    return(0);
                break;
            case 15:
                if(CurrentCounter>dv)
                    return(0);
                break;
            case 16:
                if(CurrentCounter<=dv)
                    return(0);
                break;
            case 17:
                if(Items[dv].Location!=Items[dv].InitialLoc)
                    return(0);
                break;
            case 18:
                if(Items[dv].Location==Items[dv].InitialLoc)
                    return(0);
                break;
            case 19:/* Only seen in Brian Howarth games so far */
                if(CurrentCounter!=dv)
                    return(0);
                break;
        }
        cc++;
    }
    /* Actions */
    act[0]=Actions[ct].Action[0];
    act[2]=Actions[ct].Action[1];
    act[1]=act[0]%150;
    act[3]=act[2]%150;
    act[0]/=150;
    act[2]/=150;
    cc=0;
    pptr=0;
    while(cc<4)
    {
        if(act[cc]>=1 && act[cc]<52)
        {
            Output(Messages[act[cc]]);
            Output("\n");
        }
        else if(act[cc]>101)
        {
            Output(Messages[act[cc]-50]);
            Output("\n");
        }
        else switch(act[cc])
        {
            case 0:/* NOP */
                break;
            case 52:
                if(CountCarried()==GameHeader.MaxCarry)
                {
                    if(Options&YOUARE)
                        Output("You are carrying too much.\n");
                    else
                        Output("I've too much to carry!\n");
                    break;
                }
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location= CARRIED;
                break;
            case 53:
                Redraw=1;
                Items[param[pptr++]].Location=MyLoc;
                break;
            case 54:
                Redraw=1;
                MyLoc=param[pptr++];
                break;
            case 55:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location=0;
                break;
            case 56:
                BitFlags|=1L<<DARKBIT;
                break;
            case 57:
                BitFlags&=~(1L<<DARKBIT);
                break;
            case 58:
                BitFlags|=(1L<<param[pptr++]);
                break;
            case 59:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location=0;
                break;
            case 60:
                BitFlags&=~(1L<<param[pptr++]);
                break;
            case 61:
                if(Options&YOUARE)
                    Output("You are dead.\n");
                else
                    Output("I am dead.\n");
                BitFlags&=~(1L<<DARKBIT);
                MyLoc=GameHeader.NumRooms;/* It seems to be what the code says! */
                Look();
                break;
            case 62:
            {
                /* Bug fix for some systems - before it could get parameters wrong */
                int i=param[pptr++];
                Items[i].Location=param[pptr++];
                Redraw=1;
                break;
            }
            case 63:
                // mseelye - Cleaned this up so it didn't use Fatal
doneit:         Output("The game is now over.");
                //sleep(5); // no point in sleep with getch
                printw("Press any key...");
                getch();
                Close(0);
            case 64:
                Look();
                break;
            case 65:
            {
                int ct=0;
                int n=0;
                while(ct<=GameHeader.NumItems)
                {
                    if(Items[ct].Location==GameHeader.TreasureRoom &&
                      *Items[ct].Text=='*')
                        n++;
                    ct++;
                }
                if(Options&YOUARE)
                    Output("You have stored ");
                else
                    Output("I've stored ");
                OutputNumber(n);
                Output(" treasures.  On a scale of 0 to 100, that rates ");
                OutputNumber((n*100)/GameHeader.Treasures);
                Output(".\n");
                if(n==GameHeader.Treasures)
                {
                    Output("Well done.\n");
                    goto doneit;
                }
                break;
            }
            case 66:
            {
                int ct=0;
                int f=0;
                if(Options&YOUARE)
                    Output("You are carrying:\n");
                else
                    Output("I'm carrying:\n");
                while(ct<=GameHeader.NumItems)
                {
                    if(Items[ct].Location==CARRIED)
                    {
                        if(f==1)
                        {
                            if (Options & TRS80_STYLE)
                                Output(". ");
                            else
                                Output(" - ");
                        }
                        f=1;
                        Output(Items[ct].Text);
                    }
                    ct++;
                }
                if(f==0)
                    Output("Nothing");
                Output(".\n");
                break;
            }
            case 67:
                BitFlags|=(1L<<0);
                break;
            case 68:
                BitFlags&=~(1L<<0);
                break;
            case 69:
                GameHeader.LightTime=LightRefill;
                if(Items[LIGHT_SOURCE].Location==MyLoc)
                    Redraw=1;
                Items[LIGHT_SOURCE].Location=CARRIED;
                BitFlags&=~(1L<<LIGHTOUTBIT);
                break;
            case 70:
                ClearScreen(); /* pdd. */
                OutReset();
                break;
            case 71:
                SaveGame();
                break;
            case 72:
            {
                int i1=param[pptr++];
                int i2=param[pptr++];
                int t=Items[i1].Location;
                if(t==MyLoc || Items[i2].Location==MyLoc)
                    Redraw=1;
                Items[i1].Location=Items[i2].Location;
                Items[i2].Location=t;
                break;
            }
            case 73:
                continuation=1;
                break;
            case 74:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location= CARRIED;
                break;
            case 75:
            {
                int i1,i2;
                i1=param[pptr++];
                i2=param[pptr++];
                if(Items[i1].Location==MyLoc)
                    Redraw=1;
                Items[i1].Location=Items[i2].Location;
                if(Items[i2].Location==MyLoc)
                    Redraw=1;
                break;
            }
            case 76:    /* Looking at adventure .. */
                Look();
                break;
            case 77:
                if(CurrentCounter>=0)
                    CurrentCounter--;
                break;
            case 78:
                OutputNumber(CurrentCounter);
                break;
            case 79:
                CurrentCounter=param[pptr++];
                break;
            case 80:
            {
                int t=MyLoc;
                MyLoc=SavedRoom;
                SavedRoom=t;
                Redraw=1;
                break;
            }
            case 81:
            {
                /* This is somewhat guessed. Claymorgue always
                   seems to do select counter n, thing, select counter n,
                   but uses one value that always seems to exist. Trying
                   a few options I found this gave sane results on ageing */
                int t=param[pptr++];
                int c1=CurrentCounter;
                CurrentCounter=Counters[t];
                Counters[t]=c1;
                break;
            }
            case 82:
                CurrentCounter+=param[pptr++];
                break;
            case 83:
                CurrentCounter-=param[pptr++];
                if(CurrentCounter< -1)
                    CurrentCounter= -1;
                /* Note: This seems to be needed. I don't yet
                   know if there is a maximum value to limit too */
                break;
            case 84:
                Output(NounText);
                break;
            case 85:
                Output(NounText);
                Output("\n");
                break;
            case 86:
                Output("\n");
                break;
            case 87:
            {
                /* Changed this to swap location<->roomflag[x]
                   not roomflag 0 and x */
                int p=param[pptr++];
                int sr=MyLoc;
                MyLoc=RoomSaved[p];
                RoomSaved[p]=sr;
                Redraw=1;
                break;
            }
            case 88:
                sleep(2);    /* DOC's say 2 seconds. Spectrum times at 1.5 */
                break;
            case 89:
                pptr++;
                /* SAGA draw picture n */
                /* Spectrum Seas of Blood - start combat ? */
                /* Poking this into older spectrum games causes a crash */
                break;
            default:
                fprintf(stderr,"Unknown action %d [Param begins %d %d]\n\r",
                    act[cc],param[pptr],param[pptr+1]);
                break;
        }
        cc++;
    }
    return(1+continuation);
}


int PerformActions(int vb,int no)
{
    static int disable_sysfunc=0;    /* Recursion lock */
    int d=BitFlags&(1L<<DARKBIT);
    
    int ct=0;
    int fl;
    int doagain=0;
    if(vb==1 && no == -1 )
    {
        Output("Give me a direction too.\n");
        return(0);
    }
    if(vb==1 && no>=1 && no<=6)
    {
        int nl;
        if(Items[LIGHT_SOURCE].Location==MyLoc ||
           Items[LIGHT_SOURCE].Location==CARRIED)
               d=0;
        if(d)
            Output("Dangerous to move in the dark!\n");
        nl=Rooms[MyLoc].Exits[no-1];
        if(nl!=0)
        {
            MyLoc=nl;
            Output("O.K.\n");
            Look();
            return(0);
        }
        if(d)
        {
            if(Options&YOUARE)
                Output("You fell down and broke your neck.\n");
            else
                Output("I fell down and broke my neck.\n");
            sleep(5);
            exit(0);
        }
        if(Options&YOUARE)
            Output("You can't go in that direction.\n");
        else
            Output("I can't go in that direction.\n");
        return(0);
    }
    fl= -1;
    while(ct<=GameHeader.NumActions)
    {
        int vv,nv;
        vv=Actions[ct].Vocab;
        /* Think this is now right. If a line we run has an action73
           run all following lines with vocab of 0,0 */
        if(vb!=0 && (doagain&&vv!=0))
            break;
        /* Oops.. added this minor cockup fix 1.11 */
        if(vb!=0 && !doagain && fl== 0)
            break;
        nv=vv%150;
        vv/=150;
        if((vv==vb)||(doagain&&Actions[ct].Vocab==0))
        {
            if((vv==0 && RandomPercent(nv))||doagain||
                (vv!=0 && (nv==no||nv==0)))
            {
                int f2;
                if(fl== -1)
                    fl= -2;
                if((f2=PerformLine(ct))>0)
                {
                    /* ahah finally figured it out ! */
                    fl=0;
                    if(f2==2)
                        doagain=1;
                    if(vb!=0 && doagain==0)
                        return(0);
                }
            }
        }
        ct++;
        if(Actions[ct].Vocab!=0)
            doagain=0;
    }
    if(fl!=0 && disable_sysfunc==0)
    {
        int i;
        if(Items[LIGHT_SOURCE].Location==MyLoc ||
           Items[LIGHT_SOURCE].Location==CARRIED)
               d=0;
        if(vb==10 || vb==18) // TAKE || DROP
        {
            /* Yes they really _are_ hardcoded values */
            if(vb==10) // "TAKE"
            {
                if(strcasecmp(NounText,"ALL")==0)
                {
                    int ct=0;
                    int f=0;
                    
                    if(d)
                    {
                        Output("It is dark.\n");
                        return 0;
                    }
                    while(ct<=GameHeader.NumItems)
                    {
                        if(Items[ct].Location==MyLoc && Items[ct].AutoGet!=NULL && Items[ct].AutoGet[0]!='*')
                        {
                            no=WhichWord(Items[ct].AutoGet,Nouns);
                            disable_sysfunc=1;    /* Don't recurse into auto get ! */
                            PerformActions(vb,no);    /* Recursively check each items table code */
                            disable_sysfunc=0;
                            if(CountCarried()==GameHeader.MaxCarry)
                            {
                                if(Options&YOUARE)
                                    Output("You are carrying too much.\n");
                                else
                                    Output("I've too much to carry.\n");
                                return(0);
                            }
                            Items[ct].Location= CARRIED;
                            Redraw=1;
                            OutBuf(Items[ct].Text);
                            Output(": O.K.\n");
                            f=1;
                        }
                        ct++;
                    }
                    if(f==0)
                        Output("Nothing taken.\n");
                    return(0);
                }
                if(no==-1)
                {
                    Output("What ?\n");
                    return(0);
                }
                if(CountCarried()==GameHeader.MaxCarry)
                {
                    if(Options&YOUARE)
                        Output("You are carrying too much.\n");
                    else
                        Output("I've too much to carry.\n");
                    return(0);
                }
                i=MatchUpItem(NounText,MyLoc);
                if(i==-1)
                {
                    if(Options&YOUARE)
                        Output("It is beyond your power to do that.\n");
                    else
                        Output("It's beyond my power to do that.\n");
                    return(0);
                }
                Items[i].Location= CARRIED;
                Output("O.K.\n");
                Redraw=1;
                return(0);
            }
            if(vb==18) // "DROP"
            {
                if(strcasecmp(NounText,"ALL")==0)
                {
                    int ct=0;
                    int f=0;
                    while(ct<=GameHeader.NumItems)
                    {
                        if(Items[ct].Location==CARRIED && Items[ct].AutoGet && Items[ct].AutoGet[0]!='*')
                        {
                            no=WhichWord(Items[ct].AutoGet,Nouns);
                            disable_sysfunc=1;
                            PerformActions(vb,no);
                            disable_sysfunc=0;
                            Items[ct].Location=MyLoc;
                            OutBuf(Items[ct].Text);
                            Output(": O.K.\n");
                            Redraw=1;
                            f=1;
                        }
                        ct++;
                    }
                    if(f==0)
                        Output("Nothing dropped.\n");
                    return(0);
                }
                if(no==-1)
                {
                    Output("What ?\n");
                    return(0);
                }
                i=MatchUpItem(NounText,CARRIED);
                if(i==-1)
                {
                    if(Options&YOUARE)
                        Output("It's beyond your power to do that.\n");
                    else
                        Output("It's beyond my power to do that.\n");
                    return(0);
                }
                Items[i].Location=MyLoc;
                Output("O.K.\n");
                Redraw=1;
                return(0);
            }
        }
    }
    return(fl);
}

/* 
 * mseelye - setup for ncurses windows
 * also used to resize windows on resize event
 */
void setupWindows()
{
    initscr();
    noecho();
    cbreak();
    start_color();
    use_default_colors();
    
    Width = COLS -2;
    BottomHeight = LINES - TopHeight - 2;
    int resize = 1;
    
    if(top_win == 0)
    {
        top_win = derwin(stdscr, TopHeight, Width, 1,1);
        scrollok(top_win, TRUE);
        init_pair(1, COLOR_BLACK, TOPCOL);
        wbkgd(top_win, COLOR_PAIR(1));
        werase(top_win);
        resize = 0;
    }
    if(bot_win == 0)
    {
        bot_win = derwin(stdscr, BottomHeight, Width, TopHeight+1,1);
        scrollok(bot_win, TRUE);
        init_pair(2, COLOR_WHITE, BOTCOL);
        wbkgd(bot_win, COLOR_PAIR(2));
        werase(bot_win);
        resize = 0;
    }

    if(resize != 0)
    {
        // Resize windows and refresh a bit
        wresize(top_win, TopHeight, Width-1);
        wresize(bot_win, BottomHeight, Width-1);
        refresh(); // Tries to keep the "border" black
        Look();
        wbkgdset(bot_win, '-'); // No idea why this (any char other than space?) works instead of ' '
        wbkgd(bot_win, COLOR_PAIR(2));
        wclrtobot(bot_win);
    }

    OutReset();

    wrefresh(top_win);
    wrefresh(bot_win);
}

// mseelye - void to int
int main(int argc, char *argv[])
{
    FILE *f;
    int vb,no;
    
    while(argv[1])
    {
        if(*argv[1]!='-')
            break;
        switch(argv[1][1])
        {
            case 'y':
                Options|=YOUARE;
                break;
            case 'i':
                Options&=~YOUARE;
                break;
            case 'd':
                Options|=DEBUGGING;
                break;
            case 's':
                Options|=SCOTTLIGHT;
                break;
            case 't':
                Options|=TRS80_STYLE;
                break;
            case 'p':
                Options|=PREHISTORIC_LAMP;
                break;
            case 'h':
            default:
                fprintf(stderr,"%s: [-h] [-y] [-s] [-i] [-t] [-d] [-p] <game.dat> [savefile].\n\r", argv[0]);
                fprintf(stderr," -h:help\n\r -y:'you are'\n\r -s:scottlight\n\r -i:'i am'\n\r -d:debug\n\r -p:old lamp behavior\n\r -t:trs-80 formatting\n\r game.dat:required\n\r savefile:optional\n\r");
                exit(1);
        }
        if(argv[1][2]!=0)
        {
            fprintf(stderr,"%s: option -%c does not take a parameter.\n\r",
                argv[0],argv[1][1]);
            exit(1);
        }
        argv++;
        argc--;
    }

    if(argc!=2 && argc!=3)
    {
        fprintf(stderr,"%s [-h] [-y] [-s] [-i] [-t] [-d] [-p] <game.dat> [savefile].\n\r",argv[0]);
        exit(1);
    }
    f=fopen(argv[1],"r");
    if(f==NULL)
    {
        perror(argv[1]);
        exit(1);
    }

    if (Options & TRS80_STYLE)
    {
        Width = 64;
        TopHeight = 11;
        BottomHeight = 13;
    }
    else
    {
        // mseelye - note: Starts at this, window setup changes them
        Width = 80; 
        TopHeight = 10;
        BottomHeight = 15;
    }

    DisplayUp=1;
    setupWindows(); // mseelye - setup ncurses windows

    // mseelye - updated blurb
    OutBuf("\
scottfree-ncurses {VERSION}, by Mark Seelye 2020. A reworking of...\n\
Scott Free, A Scott Adams game driver in C.\n\
Release 1.14b (PC), (c) 1993,1994,1995 Swansea University Computer Society.\n\
Distributed under the GNU software license\n\n");
    LoadDatabase(f,(Options&DEBUGGING)?1:0);
    fclose(f);
    if(argc==3)
        LoadGame(argv[2]);
    srand(time(NULL));
    Look();
    while(1)
    {
        if(Redraw!=0)
        {
            Look();
            Redraw=0;
        }
        PerformActions(0,0);
        if(Redraw!=0)
        {
            Look();
            Redraw=0;
        }
        GetInput(&vb,&no);
        switch(PerformActions(vb,no))
        {
            case -1:Output("I don't understand your command.\n");
                break;
            case -2:Output("I can't do that yet.\n");
                break;
        }
        /* Brian Howarth games seem to use -1 for forever */
        if(Items[LIGHT_SOURCE].Location/*==-1*/!=DESTROYED && GameHeader.LightTime!= -1)
        {
            GameHeader.LightTime--;
            if(GameHeader.LightTime<1)
            {
                BitFlags|=(1L<<LIGHTOUTBIT);
                if(Items[LIGHT_SOURCE].Location==CARRIED ||
                    Items[LIGHT_SOURCE].Location==MyLoc)
                {
                    if(Options&SCOTTLIGHT)
                        Output("Light has run out! ");
                    else
                        Output("Your light has run out. ");
                }
                if(Options&PREHISTORIC_LAMP)
                    Items[LIGHT_SOURCE].Location=DESTROYED;
            }
            else if(GameHeader.LightTime<25)
            {
                if(Items[LIGHT_SOURCE].Location==CARRIED ||
                    Items[LIGHT_SOURCE].Location==MyLoc)
                {
            
                    if(Options&SCOTTLIGHT)
                    {
                        Output("Light runs out in ");
                        OutputNumber(GameHeader.LightTime);
                        Output(" turns. ");
                    }
                    else
                    {
                        if(GameHeader.LightTime%5==0)
                            Output("Your light is growing dim. ");
                    }
                }
            }
        }
    }
}
