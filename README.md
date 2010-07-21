

# ScummC - A Scumm Compiler


## I. What is ScummC ?

ScummC is a set of tools allowing to create SCUMM games from scratch.
It is capable to create games for SCUMM version 6 (used by Day Of
The Tentacle, aka dott) and somewhat support version 7 (used by Full
Throttle).


## II. This release

This release mark a new important milestone: it is finally possible
to create a 100% free SCUMM game.

In this release you will find:

  * scc      : the compiler
  * sld      : the linker
  * boxedit  : a room box editor
  * cost     : a costume compiler
  * char     : a charset converter
  * costview : a costume viewer/editor prototype
  * soun     : a simple soun resources builder
  * midi     : a tool to hack MIDI files

Some utilities that might (or might not) be useful with SCUMM related
hacking.

  * zpnn2bmp : convert ZPnn blocks to bmp
  * imgsplit : split a bmp (it conserve the palette)
  * imgremap : exchange two colors (both palette and data)
  * palcat   : combine bmp palettes.
  * raw2voc  : read some raw 8bit unsigned samples and pack them in a voc.
  * scvm     : a VM prototype that should become a debugger some day.


## III. Compiling it

Requirements:

  * GNU make >= 3.80
  * bison (minimal version between 1.28 and 1.35)
  * GTK >= 2.4 (for boxedit and costview)

Optional libraries:

  * Freetype (to build charsets from ttf fonts)
  * SDL (for scvm)
  * xsltproc (for the man pages and help messages)

To compile just run configure and then make (or gmake). If you want to
also compile the extra utilities run make all. To see all available
targets run make help.

Only GNU/Linux i386 is fully tested, however in the past the code was
regularly tested on the (now defunct) sourceforge compile farm. So the
following platforms should still be fully supported:

  * GNU/Linux   i386 amd64 alpha openpower
  * NetBSD      i386
  * FreeBSD     i386
  * OpenBSD     i386
  * Solaris     i386 sparc
  * OSX         i386 ppc

Mingw is only supported via cross-compiling because the Makefiles aren't
working with MSys. Cygwin status is currently unknown.


## IV. What can i do with that ?

Already quite a lot ;) All major features from the engine are usable,
and a full game can be built from scratch. SCUMM version 6 is the default
target, but version 7 is also partially supported.


## V. Getting started

An example is provided. Just cd into one of the example directory and
run make. If all went well it should produce 3 files: scummc6.000,
scummc6.001 and scummc6.sou.

To run ScummC games with ScummVM their is two ways. The recommended
way is to patch ScummVM to have it recognise ScummC games and run them
without any game specific hack. Otherwise you can run the game as Day
of the tentacle, this is not recommended because a few hacks might
kick in. However that shouldn't be a problem with small test games.

To patch ScummVM cd in your ScummVM source directory and run:
 `patch -p1 < ~/scummc-X.X/patches/scummvm-Y.Y.Y-scummc.diff`

Alternatively you can use the original LEC interpreter, but be warned
that a few things (namely save/load) are currently not working with it.
Not that you really need to save in the example game ;)

Note that to run the game as Day of the Tentacle with ScummVM or with the
LEC interpreter you must give the following extra options to sld:
`-o tentacle -key 0x69`. For the LEC interpreter you must also rename
tentacle.sou to monster.sou.

Documentation is currently limited, the two most useful sources are:

  * http://alban.dotsec.net/Projects/ScummC

    The home of this project, for all ScummC specific documentation
    and some technical stuff about SCUMM internals.

  * http://wiki.scummvm.org/index.php/SCUMM_Technical_Reference

    More techinal documentation. If you are new to SCUMM you should at least
    read http://wiki.scummvm.org/index.php/SCUMM_Virtual_Machine which give
    a nice overview of the VM.

  * The man pages

    The man directory contain man pages for the various tools. Just open
    the XML files with any XSLT capable browser (like firefox). The lastest
    versions can also be read at: https://dotsec.net/repos/scummc/trunk/man


## VI. The box editor

This tool allow you to define the boxes in your room in a graphical
manner and to name them for easy reffering in the scripts.
Be warned that the interface is perhaps a bit unusual. The 2 open and save
buttons should be self-explicit. But both have an alternative action if
clicked with a shift key pressed. Namely open a background image and
save as. On the view you have following actions:

 * left button        : add point to the selection
 * right button       : remove point from the selection

 * shift+left button  : add box to the selection
 * shift+right button : remove box from the selection

 * ctrl+left button   : select other point (a point need to be selected first)
 * ctrl+right button  : select other box (a box need to be selected first)

 * left button drag   : move the selection
 * middle button drag : move the view

There are also the following keys:

 * b         : create a new box
 * d/suppr   : delete the selection
 * esc       : clear selection
 * u/q       : undo
 * r/o       : redo
 * >/+       : zoom in
 * </-       : zoom out
 * s         : save
 * S         : save as

I'm using a Dvorak layout so the o/q are well placed, i put u/r as
alternative because they are easy to remember. However i would be glad
to add some qzerty friendly bindings.

On the command line you can specify a box filename and/or a background image
with the -img parameter.


## VII. Warning and other legal stuff

Be warned this is still beta, some features are still missing. Use at your
own risk. All the code has been written by myself and is under GPL.
Some bit of code have been stolen (ie. copy/pasted and c-ifier) from scummvm.
I don't have time for legal stuff at the moment so just play fair ;)


## VIII. Thanks

Big thanks to all ScummVM coders and contributors. All this would
have never been possible without you !!!!

Big thanks to David Given, http://www.cowlark.com/scumm was a very
useful source.

Big thanks to sourceforge.net for the compile farm that allowed making
this software portable on 10 different platforms.

Big thanks to Jesse McGrew and James Urquhart for their contributions.

Big thanks to Gerrit Karius for OpenQuest and generally making this
project move forward.

And last but not least, thanks to the few people how at least tried to
compile and perhaps even used a bit one of those early versions.


## IX. Contact

ScummC have a project page at Gna!: http://gna.org/projects/scummc/
There you can find our mailing list, bug tracker, etc

For anything relevant to ScummC and SCUMM games developement use
the mailing list scummc-general@gna.org, for anything else I can
be reached at albeu@free.fr.

Patches and suggestions of all kinds are always welcome!
