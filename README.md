## ScottFree-ncurses - ScottFree Revision 1.14b for ncurses environments

**Source**:  [https://github.com/mseelye/scottfree-ncurses](https://github.com/mseelye/scottfree-ncurses)  
**This rework by**:  Mark Seelye <mseelye@yahoo.com>  
**Original**: ["ScottFree release 1.14b" for DOS, by Alan Cox](http://ifarchive.org/if-archive/scott-adams/interpreters/scottfree/scott.zip)  
**Copyright**:  
Scott Free, A Scott Adams game driver in C.  
Release 1.14b (PC), (c) 1993,1994,1995 Swansea University Computer Society.  
Port to ncurses Release 0.1, (c) 2020 Mark Seelye  
Distributed under the GNU software license  
**License**:   GNU GPL, version 2 - see [GPL-2.0.md](https://bitbucket.org/mseelye/scottfree-ncurses/src/master/GPL-2.0.md)  

## Credits and Thanks
* Original SCOTT.c/SCOTT.h source from "ScottFree release 1.14b" for DOS, by Alan Cox. From [ifarchive.org](http://ifarchive.org/indexes/if-archiveXscott-adamsXinterpretersXscottfree.html)  
* Mike Taylor's fixes for the **definition notes** from his [ScottKit](https://github.com/MikeTaylor/scottkit/tree/master/docs/notes) project.  
* Jason Compton, who made a great Scott Adams-format game called "Ghost King" that inspired me to play around with Scott Adams Interpreters  
* "Ghost King" is available in the /games directory.  
* ALL the games in the /games directory belong to the authors who created them. I do not claim to have authored any of them, and I provide them here for the sake of convenience. I believe almost all of these came from [ifarchive.org](http://ifarchive.org/indexes/if-archiveXscott-adamsXinterpretersXscottfree.html)  

## Goals for this Project
Before working on [scottfree64 for the Commodore 64](https://github.com/mseelye/scottfree-commodore64), I did a quick port to ncurses. I know there are other ncurses ports out there, but I thought I'd put mine out there too.
As with scottfree64, I wanted to make as few changes to the original SCOTT.C/SCOTT.H as possible, so there are no optimizations here.

## Some Notes
* You will need the ncurses development libraries installed to build this.
* If you exit the program abnormally it may make your terminal wonky, try `stty sane`
* I've tested this with Windows MSys(xterm), Mac, and Ubuntu 16.04.

## Enough, how do I play?
With ncurses libraries installed, gcc, and make - you should be able to build this. It will put scottfree-ncurses(.exe) in the out directory.

* build: `make`  
* run: `./out/scottfree-ncurses [optional arguments] games/[game dat file] [optional saved game]`  
* Example run: `./out/scottfree-ncurses -i -d games/ghostking.dat yorick`  
Where `-i and -d` are optional switches, `ghostking.dat` is the playable game file to load, and `yorick` is the optional saved game.  

The options available in this build are the same as the original ScottFree 1.14b:  

* **-h**  Shows help similar to this README.
* **-y**  Use "you are" mode.
* **-s**  Use "scottlight" mode for light/darkness messages.
* **-i**  Use "I am" mode. (Default behavior, expected by most Adams-format games)
* **-d**  Debug. Shows detailed loading progress and data about the gamefile during the load.
* **-p**  Use old or "prehistoric" lamp behavior. 

## Games
Again, all of the games in the [games](games) directory belong to the original authors. Provided here for the sake of convenience.

#### Ghost King by Jason Compton
* **[Ghost King](https://ifdb.tads.org/viewgame?id=pv6hkqi34nzn1tdy)** by [Jason Compton](http://twitter.com/jpcwrites) "Your father is dead and you're sure your uncle is responsible. You tried to tell your mother so. Instead of believing you, she married him. Now you're going to uncover the truth and set things right..."

#### Scott Adams Classic Adventures (SACA)

* **adv01.dat**      [Adventureland](https://ifdb.tads.org/viewgame?id=dy4ok8sdlut6ddj7)
* **adv02.dat**      [Pirate Adventure (aka. Pirate's Cove)](https://ifdb.tads.org/viewgame?id=zya3mo3njj58hewi)
* **adv03.dat**      [Secret Mission(aka "Mission Impossible")](https://ifdb.tads.org/viewgame?id=89kxtet3vb9lzj87)
* **adv04.dat**      [Voodoo Castle](https://ifdb.tads.org/viewgame?id=ay2jy3sc3e6s9j4k)
* **adv05.dat**      [The Count](https://ifdb.tads.org/viewgame?id=89qmjvv4cb0z93t5)
* **adv06.dat**      [Strange Odyssey](https://ifdb.tads.org/viewgame?id=025dj4on6jr2c867)
* **adv07.dat**      [Mystery Fun House](https://ifdb.tads.org/viewgame?id=cj05ocxhay4dbrfs)
* **adv08.dat**      [Pyramid of Doom](https://ifdb.tads.org/viewgame?id=hew4c6rciycb6vog)
* **adv09.dat**      [Ghost Town](https://ifdb.tads.org/viewgame?id=rlxb5i0vjrnfr6x9)
* **adv10.dat**      [Savage Island, Part I](https://ifdb.tads.org/viewgame?id=wkaibkem4nxzo53y)
* **adv11.dat**      [Savage Island, Part II](https://ifdb.tads.org/viewgame?id=aqy6km542aq20jh4)
* **adv12.dat**      [The Golden Voyage](https://ifdb.tads.org/viewgame?id=4yo4je8dh53ug9qs)
#### Scott Adams later Adventures (plus a sample game)
* **adv13.dat**      [Sorcerer of Claymorgue Castle](https://ifdb.tads.org/viewgame?id=11tnb08k1jov4hyl)
* **adv14a.dat**     [Return to Pirate's Isle](https://ifdb.tads.org/viewgame?id=rp9eibu02f9vp2sv)
* **adv14b.dat**     [Buckaroo Banzai](https://ifdb.tads.org/viewgame?id=m85x5yr0zbopyuyb)
* **sampler1.dat**   [Mini Adventure Sampler](https://ifdb.tads.org/viewgame?id=7nkd8ib4xbeqr7pm)
#### Scott Adams later Adventures Marvel(TM) Adventures (Questprobe series)    
* **quest1.dat**     [The Hulk](https://ifdb.tads.org/viewgame?id=4blbm63qfki4kf2p)
* **quest2.dat**     [Spiderman](https://ifdb.tads.org/viewgame?id=ngi8ox3s9gfcand2)

These Scott Adams original text Adventure games are still copyrighted by Scott Adams and are not Public Domain. They may be freely downloaded and enjoyed though.
Please refer to the original shares at [ifarchive.org](http://ifarchive.org/indexes/if-archiveXscott-adamsXgamesXscottfree.html) Look for AdamsGames.zip  

#### The 11 Mysterious Adventures by Brian Howarth:  
(c) 1981,82,83. All games written by Brian Howarth. #5-7 co-written by Wherner Barnes, #8 and #11 co-written by Cliff J. Ogden.  

* **1_baton.dat**                     [The Golden Baton](https://ifdb.tads.org/viewgame?id=v148gq1vx7leo8al)
* **2_timemachine.dat**               [The Time Machine](https://ifdb.tads.org/viewgame?id=g7h92i8ucy0sfll2)
* **3_arrow1.dat**                    [Arrow of Death Part 1](https://ifdb.tads.org/viewgame?id=g25uklrs45gj7e02)
* **4_arrow2.dat**                    [Arrow of Death Part 2](https://ifdb.tads.org/viewgame?id=fy9klru0b5swgnsz)
* **5_pulsar7.dat**                   [Escape from Pulsar 7](https://ifdb.tads.org/viewgame?id=tbrvwzmvgmmnavhl)
* **6_circus.dat**                    [Circus](https://ifdb.tads.org/viewgame?id=bdnprzz9zomlge4b)
* **7_feasibility.dat**               [Feasibility Experiment](https://ifdb.tads.org/viewgame?id=up2ak731a6h2quna)
* **8_akyrz.dat**                     [The Wizard of Akyrz](https://ifdb.tads.org/viewgame?id=u1kmutsdwp8uys1h)
* **9_perseus.dat**                   [Perseus and Andromeda](https://ifdb.tads.org/viewgame?id=gti5j0nqvvqqvnzo)
* **A_tenlittleindians.dat**          [Ten Little Indians](https://ifdb.tads.org/viewgame?id=z7ettlqezn4mcnng)
* **B_waxworks.dat**                  [Waxworks](https://ifdb.tads.org/viewgame?id=lkt6sm3mgarb02bo)

All of the commercially published Scott Adams-format games available in TRS-80 .dat format are expected to work on this build of ScottFree. Some modern games which play at the boundaries of the Adams specification may fail on this build due to memory limitations or other quirks.

### DAT files / ifarchive.org
**DAT** Is a format used by some of the Scott Adams game kits and players. 
You can find a lot of them on sites like:  
http://ifarchive.org/indexes/if-archiveXscott-adamsXgames.html

## Building from source
You need to have ncurses installed. This varies from platform to platform how to do it. You also need gcc (or some other gcc compatible C compiler, clang etc.), and GUN make.  

```
$ make
mkdir -p out
*** Building src/scottfree-ncurses.c
gcc -o out/scottfree-ncurses src/scottfree-ncurses.c src/scottfree.h -Wall -lncurses
*** Building complete
```

# Tools and Stuff
### ncurses
At this point I can't recall what I did to get ncurses installed on my MSys2/Mingw64 environment. You might want to start with `pacman -S ncurses` and/or `pacman -S ncurses-devel`.  
For Ubuntu and Mac, I'm fairly certain I also just used apt-get and brew. I'll check and update these docs at some point.

### gcc
Any gcc compiler should do fine, I have only really tested this with gcc and clang though.

### make
I use GNU Make 4.3 in my Windows MSys2 environment, GNU Make 4.1 in my Ubuntu environment, and GNU Make 3.8.1 on my MacBook Pro.

### Various
The Makefile also makes use of various shell commands like **echo, rm,** and **mkdir**. Hopefully these are available in your build environment.

