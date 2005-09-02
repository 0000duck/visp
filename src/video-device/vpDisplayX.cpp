
/*                                                                -*-c++-*-
    Copyright (C) 1998  IRISA-INRIA Rennes Vista Project

    Contact:
       Eric Marchand
       IRISA-INRIA Rennes
       35042 Rennes Cedex
       France

    email: marchand@irisa.fr
    www  : http://www.irisa.fr/vista

    Auteur :
      Fabien Spindler
      Eric Marchand

    Creation : 1 octobre 1998
    Revision : 11 mai 2000 - Ajout fonctionnalites profondeur d'ecran 16 bits

*/

/*!
  \file vpDisplayX.cpp
  \brief Define the X11 console to display images
*/

/*!
  \class vpDisplayX

  \brief La classe vpDisplayX permet de g�rer l'affichage d'images dans des
  fen�tres X.

  \author Fabien Spindler (Fabien.Spindler@irisa.fr), Eric Marchand
  (Eric.Marchand@irisa.fr) Irisa / Inria Rennes

  Cette classe permet d'afficher des images de type "unsigned char", d'y tracer
  des points, des segments, d'y superposer du texte et de r�cup�rer des
  �venements li�s � la souris.

  Cette classe fonctionne avec des profondeurs d'�cran de 8, 16 ou 24 bits.

*/

#include <visp/vpConfig.h>
#ifdef HAVE_LIBX11

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Display stuff
#include <visp/vpDisplay.h>
#include <visp/vpDisplayX.h>

//debug / exception
#include <visp/vpDebug.h>
#include <visp/vpDisplayException.h>

// math
#include <visp/vpMath.h>

/*!

  Ce constructeur de la classe vpDisplayX g�re l'ouverture de la fen�tre X
  permettant de visualiser des images de taille (cols x rows). Les param�tres
  \e windowXPosition et \e windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la position de la fen�tre est
  quelconque. Le param�tre \e title permet de donner un titre � la fen�tre.

*/
vpDisplayX::vpDisplayX(vpImage<unsigned char> &I,
		       int _windowXPosition,
		       int _windowYPosition,
		       char *_title) : vpDisplay()
{
  init(I,_windowXPosition,_windowYPosition, _title) ;

}


/*!

  Ce constructeur de la classe vpDisplayX g�re l'ouverture de la fen�tre X
  permettant de visualiser des images de taille (cols x rows). Les param�tres
  \e windowXPosition et \e windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la position de la fen�tre est
  quelconque. Le param�tre \e title permet de donner un titre � la fen�tre.

*/
vpDisplayX::vpDisplayX(vpImage<vpRGBa> &I,
		     int _windowXPosition,
		     int _windowYPosition,
		     char *_title)
{
  title = NULL ;
  init(I,_windowXPosition,_windowYPosition, _title) ;
}

/*!


  Ce constructeur de la classe vpDisplayX g�re l'ouverture de la fen�tre X
  permettant de visualiser des images de taille (cols x rows). Les param�tres
  \e windowXPosition et \e windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la position de la fen�tre est
  quelconque. Le param�tre \e title permet de donner un titre � la fen�tre.

*/
vpDisplayX::vpDisplayX(int _windowXPosition, int _windowYPosition, char *_title)
{
  displayHasBeenInitialized = false ;
  windowXPosition = _windowXPosition ;
  windowYPosition = _windowYPosition ;

  title = NULL ;

  if (_title != NULL)
  {
    title = new char[strlen(_title) + 1] ; // Modif Fabien le 19/04/02
    strcpy(title,_title) ;
  }

  ximage_data_init = false;

}
/*!
  Constructeur vide de la classe vpDisplayX.
  \sa init()
*/
vpDisplayX::vpDisplayX()
{
  displayHasBeenInitialized =false ;
  windowXPosition = windowYPosition = -1 ;

  title = NULL ;
  if (title != NULL)
  {
    delete [] title ;
    title = NULL ;
  }
  title = new char[1] ;
  strcpy(title,"") ;

  Xinitialise = false ;
  ximage_data_init = false;
}

/*!
  Destructeur de la classe vpDisplayX.
*/
vpDisplayX::~vpDisplayX()
{
  closeDisplay() ;
}

/*!  Cette m�thode initialise la fen�tre X permettant de visualiser des images
  de taille (cols x rows). Les param�tres \e windowXPosition et \e
  windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la
  position de la fen�tre est quelconque. Le param�tre \e title permet de donner
  un titre � la fen�tre.

  \sa DisplayImage(), closeDisplay()
*/
void
vpDisplayX::init(vpImage<unsigned char> &I, int _windowXPosition, int _windowYPosition, char *_title)
 {

  displayHasBeenInitialized =true ;


  XSizeHints	hints;
  windowXPosition = _windowXPosition ;
  windowYPosition = _windowYPosition ;
  {
    if (title != NULL)
    {
      //   TRACE(" ") ;
      delete [] title ;
      title = NULL ;
    }

    if (_title != NULL)
    {
      title = new char[strlen(_title) + 1] ;
      strcpy(title,_title) ;
    }
  }

  // Positionnement de la fenetre dans l'�cran.
  if ( (windowXPosition < 0) || (windowYPosition < 0) ) {
    hints.flags = 0;
  }
  else {
    hints.flags = USPosition;
    hints.x = windowXPosition;
    hints.y = windowYPosition;
  }


  // setup X11 --------------------------------------------------
  ncols = I.getCols();
  nrows = I.getRows();

  if ((display = XOpenDisplay (NULL)) == NULL)
  {
    ERROR_TRACE("Can't connect display on server %s.\n", XDisplayName(NULL));
    throw(vpDisplayException(vpDisplayException::connexionError,
			     "Can't connect display on server.")) ;
  }

  screen       = DefaultScreen   (display);
  lut          = DefaultColormap (display, screen);
  screen_depth = DefaultDepth    (display, screen);

  TRACE("Screen depth: %d\n", screen_depth);

  if ((window =
       XCreateSimpleWindow (display, RootWindow (display, screen),
			    windowXPosition, windowYPosition, ncols, nrows, 1,
			    BlackPixel (display, screen),
			    WhitePixel (display, screen))) == 0)
  {
    ERROR_TRACE("Can't create window." );
    throw(vpDisplayException(vpDisplayException::cannotOpenWindowError,
			     "Can't create window.")) ;
  }

  //
  // Create color table for 8 and 16 bits screen
  //
  if (screen_depth == 8) {
    XColor colval ;

    lut = XCreateColormap(display, window,
			  DefaultVisual(display, screen), AllocAll) ;
    colval.flags = DoRed | DoGreen | DoBlue ;

    for(int i = 0 ; i < 256 ; i++) {
      colval.pixel = i ;
      colval.red = 256 * i;
      colval.green = 256 * i;
      colval.blue = 256 * i;
      XStoreColor(display, lut, &colval);
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;
  }

  else if (screen_depth == 16) {
    for (int i = 0; i < 256; i ++ ) {
      color.pad = 0;
      color.red = color.green = color.blue = 256 * i;
      if (XAllocColor (display, lut, &color) == 0) {
	ERROR_TRACE("Can't allocate 256 colors. Only %d allocated.", i);
	throw(vpDisplayException(vpDisplayException::colorAllocError,
				 "Can't allocate 256 colors.")) ;
      }
      colortable[i] = color.pixel;
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;

  }

  //
  // Create colors for overlay
  //
  switch (screen_depth) {

  case 8:
    XColor colval ;

    // Couleur NOIR.
    x_color[vpColor::black] = 0;

    // Couleur BLANC.
    x_color[vpColor::white] = 255;

    // Couleur ROUGE.
    x_color[vpColor::red]= 254;
    colval.pixel  = x_color[vpColor::red] ;
    colval.red    = 256 * 255;
    colval.green  = 0;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur VERT.
    x_color[vpColor::green] = 253;
    colval.pixel  = x_color[vpColor::green];
    colval.red    = 0;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur BLEU.
    x_color[vpColor::blue] = 252;
    colval.pixel  = x_color[vpColor::blue];
    colval.red    = 0;
    colval.green  = 0;
    colval.blue   = 256 * 255;
    XStoreColor(display, lut, &colval);

    // Couleur JAUNE.
    x_color[vpColor::yellow] = 251;
    colval.pixel  = x_color[vpColor::yellow];
    colval.red    = 256 * 255;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);
    break;

  case 16:
  case 24:
    {
    color.flags = DoRed | DoGreen | DoBlue ;

    // Couleur NOIR.
    color.pad   = 0;
    color.red   = 0;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);

    x_color[vpColor::black] = color.pixel;

    // Couleur BLANC.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 256* 255;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::white] = color.pixel;

    // Couleur ROUGE.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::red] = color.pixel;

    // Couleur VERT.
    color.pad   = 0;
    color.red   = 0;
    color.green = 256*255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::green] = color.pixel;

    // Couleur BLEU.
    color.pad = 0;
    color.red = 0;
    color.green = 0;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::blue] = color.pixel;

    // Couleur JAUNE.
    color.pad = 0;
    color.red = 256 * 255;
    color.green = 256 * 255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);

    x_color[vpColor::yellow] = color.pixel;
    break;
    }
  }


  XSetStandardProperties(display, window, title, title, None, 0, 0, &hints);
  XMapWindow(display, window) ;
  // Selection des evenements.
  XSelectInput(display, window,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask);

  // Creation du contexte graphique
  values.plane_mask = AllPlanes;
  values.fill_style = FillSolid;
  values.foreground = WhitePixel(display, screen);
  values.background = BlackPixel(display, screen);
  context = XCreateGC(display, window,
		      GCPlaneMask  | GCFillStyle | GCForeground | GCBackground,
		      &values);

  if (context == NULL)
  {
    ERROR_TRACE("Can't create graphics context.");
    throw(vpDisplayException(vpDisplayException::XWindowsError,
			     "Can't create graphics context")) ;

  }

  do
    XNextEvent(display, &event);
  while (event.xany.type != Expose);

  {
    Ximage = XCreateImage (display, DefaultVisual (display, screen),
			 screen_depth, ZPixmap, 0, NULL,
			 I.getCols() , I.getRows(), XBitmapPad(display), 0);

    Ximage->data = (char *) malloc (I.getCols() * I.getRows() * Ximage->bits_per_pixel / 8);
    ximage_data_init = true;

  }
  Xinitialise = true ;
  flushTitle(title) ;
  XSync (display, 1);

  I.display = this ;
  I.initDisplay =  true ;

}

/*!
  Cette m�thode initialise la fen�tre X
  permettant de visualiser des images de taille (cols x rows). Les param�tres
  \e windowXPosition et \e windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la position de la fen�tre est
  quelconque. Le param�tre \e title permet de donner un titre � la fen�tre.

  \sa DisplayImage(), closeDisplay()
*/
void
vpDisplayX::init(vpImage<vpRGBa> &I, int _windowXPosition, int _windowYPosition, char *_title)
{

  displayHasBeenInitialized =true ;

  XSizeHints	hints;
  windowXPosition = _windowXPosition ;
  windowYPosition = _windowYPosition ;

  {
    if (title != NULL)
    {
      delete [] title ;
      title = NULL ;
    }

    if (_title != NULL)
    {
      title = new char[strlen(_title) + 1] ; // Modif Fabien le 19/04/02
      strcpy(title,_title) ;
    }
  }

  // Positionnement de la fenetre dans l'�cran.
  if ( (windowXPosition < 0) || (windowYPosition < 0) ) {
    hints.flags = 0;
  }
  else {
    hints.flags = USPosition;
    hints.x = windowXPosition;
    hints.y = windowYPosition;
  }


  // setup X11 --------------------------------------------------
  ncols = I.getCols();
  nrows = I.getRows();

  if ((display = XOpenDisplay (NULL)) == NULL) {
    ERROR_TRACE("Can't connect display on server %s.\n", XDisplayName(NULL));
    throw(vpDisplayException(vpDisplayException::connexionError,
			     "Can't connect display on server.")) ;
  }

  screen       = DefaultScreen   (display);
  lut          = DefaultColormap (display, screen);
  screen_depth = DefaultDepth    (display, screen);

  DEBUG_TRACE(1, "Screen depth: %d\n", screen_depth);

  if ((window = XCreateSimpleWindow (display, RootWindow (display, screen),
				     windowXPosition, windowYPosition, ncols, nrows, 1,
				     BlackPixel (display, screen),
				     WhitePixel (display, screen))) == 0)
  {
    ERROR_TRACE("Can't create window." );
    throw(vpDisplayException(vpDisplayException::cannotOpenWindowError,
			     "Can't create window.")) ;
  }

  //
  // Create color table for 8 and 16 bits screen
  //
  if (screen_depth == 8) {
    XColor colval ;

    lut = XCreateColormap(display, window,
			  DefaultVisual(display, screen), AllocAll) ;
    colval.flags = DoRed | DoGreen | DoBlue ;

    for(int i = 0 ; i < 256 ; i++) {
      colval.pixel = i ;
      colval.red = 256 * i;
      colval.green = 256 * i;
      colval.blue = 256 * i;
      XStoreColor(display, lut, &colval);
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;
  }

  else if (screen_depth == 16) {
    for (int i = 0; i < 256; i ++ ) {
      color.pad = 0;
      color.red = color.green = color.blue = 256 * i;
      if (XAllocColor (display, lut, &color) == 0) {
	ERROR_TRACE("Can't allocate 256 colors. Only %d allocated.", i);
	throw(vpDisplayException(vpDisplayException::colorAllocError,
				 "Can't allocate 256 colors.")) ;
      }
      colortable[i] = color.pixel;
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;

  }


  //
  // Create colors for overlay
  //
  switch (screen_depth) {

  case 8:
    XColor colval ;

    // Couleur NOIR.
    x_color[vpColor::black] = 0;

    // Couleur BLANC.
    x_color[vpColor::white] = 255;

    // Couleur ROUGE.
    x_color[vpColor::red]= 254;
    colval.pixel  = x_color[vpColor::red] ;
    colval.red    = 256 * 255;
    colval.green  = 0;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur VERT.
    x_color[vpColor::green] = 253;
    colval.pixel  = x_color[vpColor::green];
    colval.red    = 0;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur BLEU.
    x_color[vpColor::blue] = 252;
    colval.pixel  = x_color[vpColor::blue];
    colval.red    = 0;
    colval.green  = 0;
    colval.blue   = 256 * 255;
    XStoreColor(display, lut, &colval);

    // Couleur JAUNE.
    x_color[vpColor::yellow] = 251;
    colval.pixel  = x_color[vpColor::yellow];
    colval.red    = 256 * 255;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);
    break;

  case 16:
  case 24:
    {

    color.flags = DoRed | DoGreen | DoBlue ;

    // Couleur NOIR.
    color.pad   = 0;
    color.red   = 0;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::black] = color.pixel;

    // Couleur BLANC.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 256* 255;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::white] = color.pixel;

    // Couleur ROUGE.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::red] = color.pixel;

    // Couleur VERT.
    color.pad   = 0;
    color.red   = 0;
    color.green = 256*255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::green] = color.pixel;

    // Couleur BLEU.
    color.pad = 0;
    color.red = 0;
    color.green = 0;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::blue] = color.pixel;

    // Couleur JAUNE.
    color.pad = 0;
    color.red = 256 * 255;
    color.green = 256 * 255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::yellow] = color.pixel;
    break;
    }
  }
  XSetStandardProperties(display, window, title, title, None, 0, 0, &hints);
  XMapWindow(display, window) ;
  // Selection des evenements.
  XSelectInput(display, window,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask);

  // Creation du contexte graphique
  values.plane_mask = AllPlanes;
  values.fill_style = FillSolid;
  values.foreground = WhitePixel(display, screen);
  values.background = BlackPixel(display, screen);
  context = XCreateGC(display, window,
		      GCPlaneMask  | GCFillStyle | GCForeground | GCBackground,
		      &values);

  if (context == NULL) {
   ERROR_TRACE("Can't create graphics context.");
    throw(vpDisplayException(vpDisplayException::XWindowsError,
			     "Can't create graphics context")) ;
  }

  do
    XNextEvent(display, &event);
  while (event.xany.type != Expose);


  {
    Ximage = XCreateImage (display, DefaultVisual (display, screen),
			 screen_depth, ZPixmap, 0, NULL,
			 I.getCols() , I.getRows(), XBitmapPad(display), 0);


    Ximage->data = (char *) malloc (I.getCols() * I.getRows() * Ximage->bits_per_pixel / 8);
    ximage_data_init = true;

  }
  Xinitialise = true ;

  XSync (display, true);
  flushTitle(title) ;

  I.display = this ;
  I.initDisplay =  true ;
}


/*!

  Cette m�thode initialise la fen�tre X
  permettant de visualiser des images de taille (cols x rows). Les param�tres
  \e windowXPosition et \e windowYPosition permettent de positionner le coin sup�rieur gauche de la
  fen�tre. Si \e windowXPosition ou \e windowYPosition sont n�gatifs, la position de la fen�tre est
  quelconque. Le param�tre \e title permet de donner un titre � la fen�tre.

  \sa DisplayImage(), closeDisplay()
*/
void vpDisplayX::init(int cols, int rows, int _windowXPosition, int _windowYPosition, char *_title)
{

  displayHasBeenInitialized = true ;


  /* setup X11 ------------------------------------------------------------- */
  ncols = cols;
  nrows = rows;

  XSizeHints	hints;

  windowXPosition = _windowXPosition ;
  windowYPosition = _windowYPosition ;
  // Positionnement de la fenetre dans l'�cran.
  if ( (windowXPosition < 0) || (windowYPosition < 0) ) {
    hints.flags = 0;
  }
  else {
    hints.flags = USPosition;
    hints.x = windowXPosition;
    hints.y = windowYPosition;
  }


  {
    if (title != NULL)
    {
      delete [] title ;
      title = NULL ;
    }

    if (_title != NULL)
    {
      title = new char[strlen(_title) + 1] ; // Modif Fabien le 19/04/02
      strcpy(title,_title) ;
    }
  }


  if ((display = XOpenDisplay (NULL)) == NULL) {
    ERROR_TRACE("Can't connect display on server %s.\n", XDisplayName(NULL));
    throw(vpDisplayException(vpDisplayException::connexionError,
			     "Can't connect display on server.")) ;
  }

  screen       = DefaultScreen   (display);
  lut          = DefaultColormap (display, screen);
  screen_depth = DefaultDepth    (display, screen);

  TRACE("Screen depth: %d\n", screen_depth);

  if ((window = XCreateSimpleWindow (display, RootWindow (display, screen),
				     windowXPosition, windowYPosition,
				     cols, rows, 1,
				     BlackPixel (display, screen),
				     WhitePixel (display, screen))) == 0)
  {
    ERROR_TRACE("Can't create window." );
    throw(vpDisplayException(vpDisplayException::cannotOpenWindowError,
			     "Can't create window.")) ;
  }


  //
  // Create color table for 8 and 16 bits screen
  //
  if (screen_depth == 8) {
    XColor colval ;

    lut = XCreateColormap(display, window,
			  DefaultVisual(display, screen), AllocAll) ;
    colval.flags = DoRed | DoGreen | DoBlue ;

    for(int i = 0 ; i < 256 ; i++) {
      colval.pixel = i ;
      colval.red = 256 * i;
      colval.green = 256 * i;
      colval.blue = 256 * i;
      XStoreColor(display, lut, &colval);
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;
  }

  else if (screen_depth == 16) {
    for (int i = 0; i < 256; i ++ ) {
      color.pad = 0;
      color.red = color.green = color.blue = 256 * i;
      if (XAllocColor (display, lut, &color) == 0) {
	ERROR_TRACE("Can't allocate 256 colors. Only %d allocated.", i);
	throw(vpDisplayException(vpDisplayException::colorAllocError,
				 "Can't allocate 256 colors.")) ;
      }
      colortable[i] = color.pixel;
    }

    XSetWindowColormap(display, window, lut) ;
    XInstallColormap(display, lut) ;

  }


  //
  // Create colors for overlay
  //
  switch (screen_depth) {

  case 8:
    XColor colval ;

    // Couleur NOIR.
    x_color[vpColor::black] = 0;

    // Couleur BLANC.
    x_color[vpColor::white] = 255;

    // Couleur ROUGE.
    x_color[vpColor::red]= 254;
    colval.pixel  = x_color[vpColor::red] ;
    colval.red    = 256 * 255;
    colval.green  = 0;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur VERT.
    x_color[vpColor::green] = 253;
    colval.pixel  = x_color[vpColor::green];
    colval.red    = 0;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);

    // Couleur BLEU.
    x_color[vpColor::blue] = 252;
    colval.pixel  = x_color[vpColor::blue];
    colval.red    = 0;
    colval.green  = 0;
    colval.blue   = 256 * 255;
    XStoreColor(display, lut, &colval);

    // Couleur JAUNE.
    x_color[vpColor::yellow] = 251;
    colval.pixel  = x_color[vpColor::yellow];
    colval.red    = 256 * 255;
    colval.green  = 256 * 255;
    colval.blue   = 0;
    XStoreColor(display, lut, &colval);
    break;

  case 16:
  case 24:
    {
    color.flags = DoRed | DoGreen | DoBlue ;

    // Couleur NOIR.
    color.pad   = 0;
    color.red   = 0;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);

    x_color[vpColor::black] = color.pixel;

    // Couleur BLANC.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 256* 255;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::white] = color.pixel;

    // Couleur ROUGE.
    color.pad   = 0;
    color.red   = 256* 255;
    color.green = 0;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::red] = color.pixel;

    // Couleur VERT.
    color.pad   = 0;
    color.red   = 0;
    color.green = 256*255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);
    x_color[vpColor::green] = color.pixel;

    // Couleur BLEU.
    color.pad = 0;
    color.red = 0;
    color.green = 0;
    color.blue  = 256* 255;
    XAllocColor (display, lut, &color);
    x_color[vpColor::blue] = color.pixel;

    // Couleur JAUNE.
    color.pad = 0;
    color.red = 256 * 255;
    color.green = 256 * 255;
    color.blue  = 0;
    XAllocColor (display, lut, &color);

    x_color[vpColor::yellow] = color.pixel;
    break;
    }
  }

  XSetStandardProperties(display, window, title, title, None, 0, 0, &hints);
  XMapWindow(display, window) ;
  // Selection des evenements.
  XSelectInput(display, window,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask);

  /* Creation du contexte graphique */
  values.plane_mask = AllPlanes;
  values.fill_style = FillSolid;
  values.foreground = WhitePixel(display, screen);
  values.background = BlackPixel(display, screen);
  context = XCreateGC(display, window,
		      GCPlaneMask  | GCFillStyle | GCForeground | GCBackground,
		      &values);

  if (context == NULL) {
    ERROR_TRACE("Can't create graphics context.");
    throw(vpDisplayException(vpDisplayException::XWindowsError,
			     "Can't create graphics context")) ;
  }

  do
    XNextEvent(display, &event);
  while (event.xany.type != Expose);

  {
    Ximage = XCreateImage (display, DefaultVisual (display, screen),
			 screen_depth, ZPixmap, 0, NULL,
			 cols, rows, XBitmapPad(display), 0);

    Ximage->data = (char *) malloc (cols * rows * Ximage->bits_per_pixel / 8);
    ximage_data_init = true;
  }
  Xinitialise = true ;

  XSync (display, true);
  flushTitle(title) ;

}

/*!

  Cette methode g�re l'affichage de l'image \e I (cod�e sur 256 niveaux de
  gris) dans la fen�tre X ouverte � l'aide du constructeur vpDisplayX(...) ou de
  init().

  Cette m�thode retourne une erreur si la fen�tre X devant contenir l'image \e
  I n'a pas �t� initialis�e, ou si la profondeur de l'�cran n'est pas de 8, 16
  ou 24 bits.

  \warning Cette m�thode a �galement pour effet d'effacer le plan overlay.

  \sa init(), closeDisplay()
*/
void vpDisplayX::displayImage(vpImage<unsigned char> &I)
{
  unsigned char       *src_8  = NULL;
  unsigned char       *dst_8  = NULL;
  unsigned short      *dst_16 = NULL;
  unsigned char       *dst_32 = NULL;

  if (Xinitialise)
  {
    switch (screen_depth) {
    case 8: {
      src_8 = (unsigned char *) I.bitmap;
      dst_8 = (unsigned char *) Ximage->data;
      // Correction de l'image de facon a liberer les niveaux de gris
      // ROUGE, VERT, BLEU, JAUNE
      {
	int	i = 0;
	int	size = ncols * nrows;
	unsigned char	nivGris;
	unsigned char	nivGrisMax = 255 - vpColor::none;

	while(i < size){
	  nivGris = src_8[i] ;
	  if (nivGris > nivGrisMax)
	    dst_8[i] = 255;
	  else
	    dst_8[i] = nivGris;
	  i++ ;
	}
      }

      XPutImage (display, window, context, Ximage, 0, 0, 0, 0, ncols, nrows);
      break;
    }
    case 16: {
      dst_16 = (unsigned short*)Ximage->data;

      for (int i = 0; i < nrows ; i++) {
	for (int j=0 ; j < ncols; j++){
	  *(dst_16+(i*ncols+j)) = (unsigned short)colortable[I[i][j]] ;
	}
      }

      XPutImage (display, window, context, Ximage, 0, 0, 0, 0, ncols, nrows);

      break;
    }

    case 24:
    default: {
      dst_32 = (unsigned char*)Ximage->data;
      for (int i = 0; i < ncols * nrows; i++) {
	char val = I.bitmap[i];
	*(dst_32 ++) = val;	// Composante Rouge.
	*(dst_32 ++) = val;	// Composante Verte.
	*(dst_32 ++) = val;	// Composante Bleue.
	*(dst_32 ++) = val;
      }

      // Creation de la Pixmap.
      pixmap = XCreatePixmap(display, window, ncols, nrows, screen_depth);
      // Affichage de l'image dans la Pixmap.
      XPutImage(display, pixmap, context, Ximage, 0, 0, 0, 0, ncols, nrows);
      XSetWindowBackgroundPixmap(display, window, pixmap);

      XClearWindow(display, window);
      XFreePixmap(display, pixmap);
      break;
    }
    }
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

void vpDisplayX::displayImage(vpImage<vpRGBa> &I)
{

  if (Xinitialise)
  {

    switch (screen_depth) {
    case 24:
    case 32:{
      /*
       * 32-bit source, 24/32-bit destination
       */
      unsigned char       *dst_32 = NULL;
      dst_32 = (unsigned char*)Ximage->data;
      for (int i = 0; i < I.getCols() * I.getRows() ; i++) {
	dst_32[i*4] = I.bitmap[i].B;
	dst_32[i*4 + 1] = I.bitmap[i].G;
	dst_32[i*4 + 2] = I.bitmap[i].R;
	dst_32[i*4 + 3] = I.bitmap[i].A;

      }
      break;
    }
    default:
      ERROR_TRACE("Unsupported depth (%d bpp) for color display",
		  screen_depth) ;
      throw(vpDisplayException(vpDisplayException::depthNotSupportedError,
			       "Unsupported depth for color display")) ;
    }

    XPutImage (display, window, context, Ximage, 0, 0, 0, 0,
	       I.getCols(), I.getRows());

  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

void vpDisplayX::getImage(vpImage<vpRGBa> &I)
{

  if (Xinitialise)
  {


    XImage *xi ;
    xi= XGetImage(display,window, 0,0, getCols(), getRows(), AllPlanes, ZPixmap) ;
    try{
      I.resize(getRows(), getCols()) ;
    }
    catch(...)
    {
      ERROR_TRACE(" ") ;
      throw ;
    }

    unsigned char       *src_24 = NULL;
    src_24 = (unsigned char*)xi->data;
    for (int i = 0; i < I.getCols() * I.getRows() ; i++)
    {
      I.bitmap[i].B = src_24[i*4]  ;
      I.bitmap[i].G = src_24[i*4 + 1]  ;
      I.bitmap[i].R = src_24[i*4 + 2]  ;
    }
    XDestroyImage(xi) ;

  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Cette methode g�re l'affichage de l'image \e I (cod�e sur 256 niveaux de
  gris) dans la fen�tre X ouverte � l'aide du constructeur vpDisplayX(...) ou de
  init().

  Cette m�thode retourne une erreur si la fen�tre X devant contenir l'image \e
  I n'a pas �t� initialis�e, ou si la profondeur de l'�cran n'est pas de 8, 16
  ou 24 bits.

  \warning Cette m�thode a �galement pour effet d'effacer le plan overlay.

  \sa init(), closeDisplay()
*/
void vpDisplayX::displayImage(unsigned char *I)
{

  unsigned char       *dst_32 = NULL;



  if (Xinitialise)
  {

    dst_32 = (unsigned char*)Ximage->data;

    for (int i = 0; i < ncols * nrows; i++) {
      char val = I[i];
      *(dst_32 ++) = val;	// Composante Rouge.
      *(dst_32 ++) = val;	// Composante Vertee.
      *(dst_32 ++) = val;	// Composante Bleue.
      *(dst_32 ++) = val;	// Composante Bleue.
    }

    // Creation de la Pixmap.
    pixmap = XCreatePixmap(display, window, ncols, nrows, screen_depth);
    // Affichage de l'image dans la Pixmap.
    XPutImage(display, pixmap, context, Ximage, 0, 0, 0, 0, ncols, nrows);
    XSetWindowBackgroundPixmap(display, window, pixmap);

    XClearWindow(display, window);
    XFreePixmap(display, pixmap);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  G�re la fermeture de la fenetre X ouverte � l'aide de init().

  \sa init()

*/
void vpDisplayX::closeDisplay()
{
  if (Xinitialise)
  {
    if (ximage_data_init == true)
      free(Ximage->data);

    Ximage->data = NULL;
    XDestroyImage(Ximage);

    XFreeGC(display, context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    Xinitialise = false;
    if (title != NULL)    {
      delete [] title ;
      title = NULL ;
    }

  }
  else
  {
    if (title != NULL)    {
      delete [] title ;
      title = NULL ;
    }
  }
}


/*!
  Force l'ex�cution des op�rations bufferis�es dans la fen�tre X.

  Cette m�thode renvoie OK en cas de succ�s. Elle renvoie l'erreur
  DISPLAY_ERROR si la fen�tre X n'est pas initialis�e.

  \sa init */
void vpDisplayX::flushDisplay()
{
  if (Xinitialise)
  {
    XFlush(display);
    XSync(display,1);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}


/*!

  Rempli la fenetre X de la couleur \e c. Les couleurs disponibles sont NOIR,
  BLANC, ROUGE, VERT, BLEU et JAUNE.

  Cette m�thode renvoie OK en cas de succ�s. Elle renvoie l'erreur
  DISPLAY_ERROR si la fen�tre X n'est pas initialis�e.

  \sa init
*/
void vpDisplayX::clearDisplay(int c)
{
  if (Xinitialise)
  {
    XSetWindowBackground (display, window, x_color[c]);
    XClearWindow(display, window);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Trace un point de cordonn�e (\e i, \e j) et de couleur \e col sur le plan
  overlay de la fen�tre X. Les couleurs disponibles sont NOIR, BLANC, ROUGE,
  VERT, BLEU et JAUNE. Le param�tre \e i correspond au lignes. \e j correspond
  aux colonnes.

  Cette m�thode renvoie OK en cas de succ�s. Elle renvoie l'erreur
  DISPLAY_ERROR si la fen�tre X n'est pas initialis�e.

  \sa init

*/
void vpDisplayX::displayPoint(int i, int j, int col)
{
  if (Xinitialise)
  {
    XSetForeground (display, context, x_color[col]);
    XDrawPoint(display, window,context, j, i) ;
  }
 else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Trace un segment de couleur \e col et d'�paisseur \e e sur le plan overlay de
  la fen�tre X. Les couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU et
  JAUNE.  Les points de coordonn�es (\e i1, \e j1) et (\e i2, \e j2)
  constituent les extr�mit�s du segment. Les param�tres \e i1 et \e i2
  correspondent � des lignes. Les param�tres \e j1 et \e j2 correspondent � des
  colonnes.

  \sa init

*/
void vpDisplayX::displayLine(int i1, int j1, int i2, int j2, int col, int e)
{
  if (Xinitialise)
  {
    if (e == 1) e = 0;

    XSetForeground (display, context, x_color[col]);
    XSetLineAttributes (display, context, e,
			LineSolid, CapButt, JoinBevel);

    XDrawLine (display, window,context, j1, i1, j2, i2);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Trace un segment en pointill�s de couleur \e col et d'�paisseur \e e sur le
  plan overlay de la fen�tre X. Les couleurs disponibles sont NOIR, BLANC,
  ROUGE, VERT, BLEU et JAUNE.  Les points de coordonn�es (\e i1, \e j1) et (\e
  i2, \e j2) constituent les extr�mit�s du segment. Les param�tres \e i1 et \e
  i2 correspondent � des lignes. Les param�tres \e j1 et \e j2 correspondent �
  des colonnes.

*/
void vpDisplayX::displayDotLine(int i1, int j1, int i2, int j2, int col, int e)
{

  if (Xinitialise)
  {
    if (e == 1) e = 0;

    XSetForeground (display, context, x_color[col]);
    XSetLineAttributes (display, context, e,
			LineOnOffDash, CapButt, JoinBevel);

    XDrawLine (display, window,context, j1, i1, j2, i2);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Trace une croix de couleur \e col et d'�paisseur 1 pixel sur le plan overlay
  de la fen�tre X. Les couleurs disponibles sont NOIR, BLANC, ROUGE, VERT,
  BLEU et JAUNE.  Le centre de la croix est positionn� au point de coordonn�e
  (\e i, \e j). Le param�tre \e i correspond au lignes, \e j correspond aux
  colonnes.


*/
void vpDisplayX::displayCross(int i,int j, int size,int col)
{
  if (Xinitialise)
  {
    try{
      displayLine(i-size/2,j,i+size/2,j,col,1) ;
      displayLine(i ,j-size/2,i,j+size/2,col,1) ;
    }
    catch(...)
    {
      ERROR_TRACE(" ") ;
      throw ;
    }
  }

  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }

}

/*!

  Trace une croix de couleur \e col et d'�paisseur 3 pixel sur le plan overlay
  de la fen�tre X. Les couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU
  et JAUNE.  Le centre de la croix est positionn� au point de coordonn�e (\e i,
  \e j). Le param�tre \e i correspond au lignes, \e j correspond aux colonnes.

*/
void vpDisplayX::displayCrossLarge(int i,int j, int size,int col)
{
  if (Xinitialise)
  {
    try{
      displayLine(i-size/2,j,i+size/2,j,col,3) ;
      displayLine(i ,j-size/2,i,j+size/2,col,3) ;
    }
    catch(...)
    {
      ERROR_TRACE(" ") ;
      throw ;
    }
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;    //
  }
}


/*!

  Trace une fl�che de couleur \e col sur le plan overlay de la fen�tre X. Les
  couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU et JAUNE.

*/
void vpDisplayX::displayArrow(int i1,int j1, int i2, int j2, int col, int L,int l)
{
  if (Xinitialise)
  {
    try{
      double a = i2 - i1 ;
      double b = j2 - j1 ;
      double lg = sqrt(vpMath::sqr(a)+vpMath::sqr(b)) ;

      if ((a==0)&&(b==0))
      {
	// DisplayCrossLarge(i1,j1,3,col) ;
      }
      else
      {
	a /= lg ;
	b /= lg ;

	double i3,j3  ;
	i3 = i2 - L*a ;
	j3 = j2 - L*b ;


	double i4,j4 ;

	double t = 0 ;
	while (t<=l)
	{
	  i4 = i3 - b*t ;
	  j4 = j3 + a*t ;

	  displayLine((int)i2,(int)j2,(int)i4,(int)j4,col) ;
	  t+=0.1 ;
	}
	t = 0 ;
	while (t>= -l)
	{
	  i4 = i3 - b*t ;
	  j4 = j3 + a*t ;

	  displayLine((int)i2,(int)j2,(int)i4,(int)j4,col) ;
	  t-=0.1 ;
	}
	displayLine(i1,j1,i2,j2,col) ;

      }
    }
    catch(...)
    {
      ERROR_TRACE(" ") ;
      throw ;
    }
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  Trace un rectangle de couleur \e col sur le plan overlay de la fen�tre X. Les
  couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU et JAUNE.  Le sommet
  sup�rieur gauche du carr� a pour coordonn�es (\e i, \e j). Le param�tre \e i
  correspond au lignes, \e j correspond aux colonnes. La largeur du rectangle
  est sp�cifi�e par \e width et la hauteur par \e height.

  Cette m�thode renvoie OK en cas de succ�s. Elle renvoie l'erreur
  DISPLAY_ERROR si la fen�tre X n'est pas initialis�e.


*/
void
vpDisplayX::displayRectangle(int i, int j, int width, int height, int col)
{
  if (Xinitialise)
  {
    XSetForeground (display, context, x_color[col]);
    XSetLineAttributes (display, context, 0,
			LineSolid, CapButt, JoinBevel);

    XDrawRectangle (display, window, context,  j, i, width, height);
  }
 else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}


/*!

  Affiche le texte \e string avec la couleur \e col sur le plan overlay de la
  fen�tre X. Les couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU et
  JAUNE.  Le premier carat�re du texte occupe la position (\e i, \e j). Le
  param�tre \e i correspond au lignes, \e j correspond aux colonnes.


*/
void vpDisplayX::displayCharString(int i, int j, char *string, int col)
{
  if (Xinitialise)
  {
    XSetForeground (display, context, x_color[col]);
    XDrawString( display, window, context, j, i, string, strlen(string) );
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  R�cup�re la position du dernier clic de souris dans la fen�tre X. Le point
  cliqu� a pour coordonn�es (\e i, \e j). Le param�tre \e i correspond au
  lignes, \e j correspond aux colonnes.


*/
bool
vpDisplayX::getClick(int& i, int& j)
{

  if (Xinitialise)
  {
    int x,y ;
    Window	rootwin, childwin ;
    int		root_x, root_y, win_x, win_y ;
    unsigned int	modifier ;
    // Test d'�v�nements.
    if ( XPending(display) )  {
      XNextEvent(display, &event);

      /* Detection de l'appui sur l'un des bouton de la souris. */
      switch(event.type) {

      case ButtonPress: {
	/* Recuperation de la coordonnee du pixel cliqu�.	*/
	if(XQueryPointer(display,
			 window,
			 &rootwin, &childwin,
			 &root_x, &root_y,
			 &win_x, &win_y,
			 &modifier)) {
	  x = event.xbutton.x;
	  y = event.xbutton.y;
	  i = y ;
	  j = x ;
	}
	return true ;
	break;

      } /* Fin case ButtonPress	*/
      } /* Fin switch type d'evenement.	*/
    }
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
    return false ;
}


/*!

  Attend un clic de souris dans la fen�tre X.

  Cette m�thode renvoie OK en cas de succ�s. Elle renvoie l'erreur
  DISPLAY_ERROR si la fen�tre X n'est pas initialis�e.

  \sa init
*/
void
vpDisplayX::getClick()
{

  if (Xinitialise)
  {
    bool ret = false;
    while (ret==false)
    {
      Window	rootwin, childwin ;
      int		root_x, root_y, win_x, win_y ;
      unsigned int	modifier ;

      // Test d'�v�nements.
      if ( XPending(display) )
      {

	XNextEvent(display, &event);

	/* Detection de l'appui sur l'un des bouton de la souris. */
	switch(event.type)
	{

	case ButtonPress:
	  {
	    /* Recuperation de la coordonnee du pixel cliqu�.	*/
	    if(XQueryPointer(display,
			     window,
			     &rootwin, &childwin,
			     &root_x, &root_y,
			     &win_x, &win_y,
			     &modifier)){}
	    ret = true ;
	    break;
	  } /* Fin case ButtonPress	*/
	} /* Fin switch type d'evenement.	*/
      }
    }
    //   return(OK);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

  R�cup�re la position du dernier clic de souris dans la fen�tre X et le num�ro
  du bouton utilis� pour r�aliser ce clic. Le point cliqu� a pour coordonn�es
  (\e i, \e j). Le param�tre \e i correspond au lignes, \e j correspond aux
  colonnes. Le param�tre \e button peut prendre les valeurs 1 (bouton gauche de
  la souris), 2 (bouton du milieu) ou encore 3 (bouton droit).

  Cette m�thode renvoie OK si un clic de souris a �t� d�tect�. Elle renvoie
  l'erreur DISPLAY_ERROR si la fen�tre X n'est pas initialis�e, ou si aucun
  clic de souris n'a �t� d�tect�.

  \sa init

*/
bool
vpDisplayX::getClick(int& i, int& j, int& button)
{

  if (Xinitialise)
  {
    int x,y ;
    Window	rootwin, childwin ;
    int		root_x, root_y, win_x, win_y ;
    unsigned int	modifier ;

    // Test d'�v�nements.
    if ( XPending(display) )  {

      XNextEvent(display, &event);

      /* Detection de l'appui sur l'un des bouton de la souris. */
      switch(event.type) {

      case ButtonPress: {
	/* Recuperation de la coordonnee du pixel cliqu�.	*/
	if(XQueryPointer(display,
			 window,
			 &rootwin, &childwin,
			 &root_x, &root_y,
			 &win_x, &win_y,
			 &modifier)) {
	  x = event.xbutton.x;
	  y = event.xbutton.y;
	  i = y ;
	  j = x ;
	  switch(event.xbutton.button)
	  {
	  case Button1: button = vpDisplay::button1; break;
	  case Button2: button = vpDisplay::button2; break;
	  case Button3: button = vpDisplay::button3; break;
	  }
	}
	return true;
	break;

      } /* Fin case ButtonPress	*/
      } /* Fin switch type d'evenement.	*/
    }
  }
 else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
    return  false;
}

/*!

  R�cup�re la position du dernier clic de souris dans la fen�tre X et le num�ro
  du bouton utilis� pour r�aliser ce clic. Le point cliqu� a pour coordonn�es
  (\e i, \e j). Le param�tre \e i correspond au lignes, \e j correspond aux
  colonnes. Le param�tre \e button peut prendre les valeurs 1 (bouton gauche de
  la souris), 2 (bouton du milieu) ou encore 3 (bouton droit).

  Cette m�thode renvoie OK si un clic de souris a �t� d�tect�. Elle renvoie
  l'erreur DISPLAY_ERROR si la fen�tre X n'est pas initialis�e, ou si aucun
  clic de souris n'a �t� d�tect�.

  \sa init

*/
bool
vpDisplayX::getClickUp(int& i, int& j, int& button)
{

  if (Xinitialise)
  {
    int x,y ;
    Window	rootwin, childwin ;
    int		root_x, root_y, win_x, win_y ;
    unsigned int	modifier ;

    // Test d'�v�nements.
    if ( XPending(display) )  {

      XNextEvent(display, &event);

      /* Detection de l'appui sur l'un des bouton de la souris. */
      switch(event.type) {

      case ButtonRelease: {
	/* Recuperation de la coordonnee du pixel cliqu�.	*/
	if(XQueryPointer(display,
			 window,
			 &rootwin, &childwin,
			 &root_x, &root_y,
			 &win_x, &win_y,
			 &modifier)) {
	  x = event.xbutton.x;
	  y = event.xbutton.y;
	  i = y ;
	  j = x ;
	  switch(event.xbutton.button)
	  {
	  case Button1: button = vpDisplay::button1; break;
	  case Button2: button = vpDisplay::button2; break;
	  case Button3: button = vpDisplay::button3; break;
	  }
	}
	return true ;
	break;

      } /* Fin case ButtonPress	*/
      } /* Fin switch type d'evenement.	*/
    }
  }
 else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
  return  false ;
}

/*!

  Renvoie la profondeur de l'�cran (8, 16 ou 24 bits).
*/
int vpDisplayX::getScreenDepth()
{
  Display	*display;
  int		screen;
  int		depth;

  if ((display = XOpenDisplay(NULL)) == NULL) {
    ERROR_TRACE("Can't connect display on server %s.",
	    XDisplayName(NULL));
    throw(vpDisplayException(vpDisplayException::connexionError,
			     "Can't connect display on server.")) ;
  }
  screen = DefaultScreen(display);
  depth  = DefaultDepth(display, screen);

  XCloseDisplay(display);

  return (depth);
}

/*!

  R�cup�re la taille de l'�cran. Le param�tre \e xsize correspond au nombre de
  colonnes et \e ysize au nombre de lignes.

 */
void vpDisplayX::getScreenSize(int *xsize, int *ysize)
{
  Display	*display;
  int		screen;

  if ((display = XOpenDisplay(NULL)) == NULL)
  {
    ERROR_TRACE("Can't connect display on server %s.",
		XDisplayName(NULL));
    throw(vpDisplayException(vpDisplayException::connexionError,
			     "Can't connect display on server.")) ;
  }
  screen = DefaultScreen(display);
  *xsize = DisplayWidth (display, screen);
  *ysize = DisplayHeight(display, screen);

  XCloseDisplay(display);
}





/*!
 */
void
vpDisplayX::flushTitle(const char *windowtitle)
{
  if (Xinitialise)
  {
    XStoreName (display, window, windowtitle);
  }
  else
  {
    ERROR_TRACE("X not initialized " ) ;
    throw(vpDisplayException(vpDisplayException::notInitializedError,
			     "X not initialized")) ;
  }
}

/*!

   Trace un circle de couleur \e c sur le plan overlay de la fen�tre X. Les
   couleurs disponibles sont NOIR, BLANC, ROUGE, VERT, BLEU et JAUNE.  Le
   centre du cercle a pour coordonn�es (\e i, \e j). Le param�tre \e i
   correspond aux lignes, \e j correspond aux colonnes. Le rayon est sp�cifi�e
   par \e r.


*/
void vpDisplayX::displayCircle(int i, int j, int r, int c)
{
   if (Xinitialise)
   {
     XSetForeground (display, context, x_color[c]);
     XSetLineAttributes (display, context, 0,
                        LineSolid, CapButt, JoinBevel);

     XDrawArc (display, window, context,  j-r, i-r, r*2, r*2, 0, 360*64);
   }
  else
   {
     ERROR_TRACE("X not initialized " ) ;
     throw(vpDisplayException(vpDisplayException::notInitializedError,
			      "X not initialized")) ;
   }
}

#endif // HAVE_LIBX11
