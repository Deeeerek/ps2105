#!/usr/bin/env python

import wx;
import random;
import sys;
import getopt;
import PicoScope;

# global variables
OSCOPE_LENGTH_X           = 720;
OSCOPE_LENGTH_Y           = 460;
fakePicoScope             = False;
numberOfDivisions_time    = 30
numberOfDivisions_voltage = 30
voltageChoices            = [ 0.100, 0.200, 0.500, 1, 2, 5, 10, 20 ];

class MyFrame( wx.Frame ):
    def __init__( self, parent ):
        wx.Frame.__init__( self,
                           parent,
                           id    = -1,
                           title = "PicoScope Oscilloscope",
                           size  = (860,530),
                           pos   = (0,20),
                           style = wx.MINIMIZE_BOX | wx.SYSTEM_MENU  | wx.CLOSE_BOX
                         );
        self.buttonRun = wx.Button( parent=self,
                                    id=-1,
                                    label="Run",
                                    pos=wx.Point(10, 20),
                                    size=(100,20) );

        self.buttonStop = wx.Button( parent=self,
                                     id=-1,
                                     label="Stop",
                                     pos=wx.Point(10, 50),
                                     size=(100,20) );

        self.waveBitmap = wx.StaticBitmap( parent = self,
                                           pos    = (120,20),
                                           size   = (OSCOPE_LENGTH_X,OSCOPE_LENGTH_Y),
                                           style  = 0,
                                           bitmap = wx.EmptyBitmap(OSCOPE_LENGTH_X,OSCOPE_LENGTH_Y)
                                         );

        self.timeBase = wx.Choice( parent  = self,
                                   pos     = (10,80),
                                   size    = (100,20),
                                   choices = ["10ns","20ns","40ns","80ns","160ns","320ns","640ns","1.024us","2.048us","4.096us",
                                              "8.192us","16.384us","32.768us","65.536us","131.07us","262.14us","524.29us","1.048ms","2.097ms","4.194ms",],
                                 );

        self.voltageRange = wx.Choice( parent  = self,
                                       pos     = (10,110),
                                       size    = (100,20),
                                       choices = [ "100 mV", "200 mV", "500 mV", "1 V", "2 V", "5 V", "10 V", "20 V" ],
                                     );
        self.couplingType = wx.Choice( parent  = self,
                                       pos     = (10,140),
                                       size    = (100,20),
                                       choices = [ "DC", "AC" ],
                                     );
        self.triggerType = wx.Choice( parent  = self,
                                      pos     = (10,170),
                                      size    = (100,20),
                                      choices = [ "None", "Rising", "Falling" ],
                                    );
        self.deltaLines = wx.Choice( parent  = self,
                                      pos     = (10,230),
                                      size    = (100,20),
                                      choices = [ "Delta-X", "Delta-Y" ],
                                    );

        self.axisLabels = wx.StaticText( parent  = self,
                                         pos     = (790-200,485),
                                         size    = (100,20),
                                         label   = "--",
                                       );
        self.cursorPosition= wx.StaticText( parent  = self,
                                            pos     = (790-500,485),
                                            size    = (100,20),
                                            label   = "",
                                          );


        self.timeBase.Select(14);
        self.timeBase.Bind(wx.EVT_CHOICE,self.changeAxis);

        self.voltageRange.Select(7);
        self.voltageRange.Bind(wx.EVT_CHOICE,self.changeAxis);

        self.couplingType.Select(1);
        self.couplingType.Bind(wx.EVT_CHOICE,self.selectCouplingType);

        self.buttonRun.Bind(wx.EVT_BUTTON,self.OnButtonRunClick);
        self.buttonStop.Bind(wx.EVT_BUTTON,self.OnButtonStopClick);

        self.waveBitmap.Bind(wx.EVT_MOTION,self.MouseMovement);
        self.waveBitmap.Bind(wx.EVT_LEAVE_WINDOW,self.MouseLeaveWindow);
        self.waveBitmap.Bind(wx.EVT_LEFT_UP,self.BitmapMouseClick);

        self.deltaLines.Bind(wx.EVT_CHOICE,self.DeltaLinesClick);

        self.myTimer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.OnTimer)
        self.changeAxis( False )

        self.SetBackgroundColour('Medium Grey')

        self.selectDeltaLine = 0  # 0: left, 1: right
        self.drawDeltaLines  = 0
        self.deltaLinesPnt   = [ 0, 0 ]
        self.lines = [];
        self.drawWave();


    def DeltaLinesClick( self, event ):
        self.drawDeltaLines  = 0
        self.selectDeltaLine = 0
        self.drawWave()

    def TranslateXY( self, X, Y ):
        Xpos     = X;
        Ypos     = ( (OSCOPE_LENGTH_Y/2) - Y ) / float(OSCOPE_LENGTH_Y/2);
        timeBase = 10e-9 * (1<<self.timeBase.GetCurrentSelection());
        X_value  = Xpos*timeBase;
        Y_value  = voltageChoices[ self.voltageRange.GetCurrentSelection() ] * Ypos;
        return (X_value, Y_value)

    def BitmapMouseClick( self, event ):
        self.drawDeltaLines = 1+self.drawDeltaLines
        if (3==self.drawDeltaLines):
            self.drawDeltaLines = 1
        if self.deltaLines.GetCurrentSelection()==0 : # delta-x
            self.deltaLinesPnt[ self.selectDeltaLine ] = event.GetX()
        else :
            self.deltaLinesPnt[ self.selectDeltaLine ] = event.GetY()
        self.selectDeltaLine = (self.selectDeltaLine+1)%2
        self.drawWave();
        
    def MouseLeaveWindow( self, event ):
        self.cursorPosition.SetLabel( "" );

    def MouseMovement( self, event ):
        (X_value, Y_value) = self.TranslateXY( event.GetX(), event.GetY() );
        self.cursorPosition.SetLabel( "(%.2e sec, %.2e V)" % (X_value,Y_value) );

    def changeAxis( self, event ):
        timeBase = 10e-9 * (1<<self.timeBase.GetCurrentSelection());
        x_tics = numberOfDivisions_time * timeBase;
        y_tics = numberOfDivisions_time * ( float(2.0*voltageChoices[ self.voltageRange.GetCurrentSelection() ]) / OSCOPE_LENGTH_Y );
        self.axisLabels.SetLabel( "Xscale:%.2e sec, Yscale:%.2e V" % (x_tics,y_tics) );

    def selectCouplingType( self, event ):
        print "New Coupling Type ",
        if (0==event.GetSelection()) :
            print "DC"
        else :
            print "AC";

    def OnTimer( self, event ):
        selectedTimeBase     = self.timeBase.GetCurrentSelection();
        selectedVoltageRange = self.voltageRange.GetCurrentSelection();
        couplingType         = self.couplingType.GetCurrentSelection();

        if False==fakePicoScope:
            ret = PicoScope.set_channel(self.myUnit,             # handle
                                        0,                       # channel
                                        1,                       # enabled
                                        couplingType,            # dc-coupling
                                        selectedVoltageRange );  # range
            ret = PicoScope.run_block(self.myUnit,               # handle
                                      OSCOPE_LENGTH_X,           # numberOfValues
                                      selectedTimeBase,          # timebase
                                      0);                        # oversample
            while( not PicoScope.ready(self.myUnit) ):
                PicoScope.delay(1);
            self.lines = PicoScope.get_values(self.myUnit,OSCOPE_LENGTH_X);
        else:
            self.lines = map( lambda x: 0, range(OSCOPE_LENGTH_X) )
            self.lines[0] = random.randint(-32767,+32767)
            for ii in range(OSCOPE_LENGTH_X-1):
                r = random.randint(-3000,3000)
                if self.lines[ii]+r > 32767 :
                    self.lines[ii+1] = self.lines[ii]-r
                elif self.lines[ii]+r < -32767 :
                    self.lines[ii+1] = self.lines[ii]-r
                else :
                    self.lines[ii+1] = self.lines[ii] + r
        self.drawWave();
        self.myTimer.Start(150,oneShot=True);

    def OnButtonRunClick( self, event ):
        if False==fakePicoScope:
            self.myUnit = PicoScope.open_unit();
            if( 0!=PicoScope.set_channel(self.myUnit,  # handle
                                      0,               # channel
                                      1,               # enabled
                                      1,               # dc-coupling
                                      7 ) ):           # range
                print "ERROR: could not set channel";

            if( 0!=PicoScope.set_trigger(self.myUnit,  # handle
                                         5,            # source
                                         0,            # threshold
                                         2,            # direction (disabled)
                                         0,            # delay
                                         0) ):         # autoTrigger
                print "ERROR: could not set trigger";
        self.myTimer.Start(150,oneShot=True);

    def OnButtonStopClick( self, event ):
        self.myTimer.Stop();
        if False==fakePicoScope:
            PicoScope.close_unit(self.myUnit);

    def prepareGrid( self ):
        mySize = self.waveBitmap.GetClientSize();
        buffer = wx.EmptyBitmap(*mySize);
        dc     = wx.MemoryDC(buffer);
        dc.Clear();
        return dc;

    def drawMarkers( self, dc ):
        if self.deltaLines.GetCurrentSelection()==0:
            if self.drawDeltaLines>=1:
                dc.SetPen(wx.Pen("green",3));
                dc.DrawLine(self.deltaLinesPnt[0],0,self.deltaLinesPnt[0],OSCOPE_LENGTH_Y)
            if self.drawDeltaLines>=2:
                dc.SetPen(wx.Pen("green",3));
                dc.DrawLine(self.deltaLinesPnt[1],0,self.deltaLinesPnt[1],OSCOPE_LENGTH_Y)
        else :
            if self.drawDeltaLines>=1:
                dc.SetPen(wx.Pen("green",3));
                dc.DrawLine(0,self.deltaLinesPnt[0],OSCOPE_LENGTH_X,self.deltaLinesPnt[0])
            if self.drawDeltaLines>=2:
                dc.SetPen(wx.Pen("green",3));
                dc.DrawLine(0,self.deltaLinesPnt[1],OSCOPE_LENGTH_X,self.deltaLinesPnt[1])
        dc.SetPen(wx.RED_PEN);
        for ii in range( OSCOPE_LENGTH_X/numberOfDivisions_time ):
            dc.SetPen(wx.LIGHT_GREY_PEN);
            dc.DrawLine(0+numberOfDivisions_time*ii,0,0+numberOfDivisions_time*ii,OSCOPE_LENGTH_Y);
            dc.SetPen(wx.RED_PEN);
            dc.DrawLine(0+numberOfDivisions_time*ii,OSCOPE_LENGTH_Y/2-3,0+numberOfDivisions_time*ii,OSCOPE_LENGTH_Y/2+3);
        for ii in range( OSCOPE_LENGTH_Y/numberOfDivisions_voltage):
            Y = (OSCOPE_LENGTH_Y/2) - (OSCOPE_LENGTH_Y/(2*numberOfDivisions_voltage))*numberOfDivisions_voltage
            dc.SetPen(wx.LIGHT_GREY_PEN);
            dc.DrawLine(0,Y+ii*numberOfDivisions_voltage,OSCOPE_LENGTH_X,Y+ii*numberOfDivisions_voltage);
            dc.SetPen(wx.RED_PEN);
            dc.DrawLine(0,Y+ii*numberOfDivisions_voltage,7,Y+ii*numberOfDivisions_voltage);
        dc.SetPen(wx.RED_PEN);
        dc.DrawLine(0,OSCOPE_LENGTH_Y/2,OSCOPE_LENGTH_X,OSCOPE_LENGTH_Y/2);


    def drawWave( self ):
        dc = self.prepareGrid();
        ii = 0;
        dc.SetPen(wx.Pen("blue",2));
        for jj in range( len(self.lines) ):
            if( jj>0 ):
                yPrevious = self.lines[jj-1];
                yNew      = self.lines[jj];
                xPrevious = ii - 1;
                xNew      = ii;
                scale     = 200.0/32768.0;
                dc.DrawLine( x1 = xPrevious,
                             y1 = OSCOPE_LENGTH_Y/2 - int(scale*yPrevious),
                             x2 = xNew,
                             y2 = OSCOPE_LENGTH_Y/2 - int(scale*yNew) );
            ii = ii + 1;
        self.drawMarkers(dc);
        self.waveBitmap.SetBitmap(dc.GetAsBitmap());

def usage():
    print "";
    print "(C) 2009, Tiago Gasiba";
    print "          PicoScope - Oscilloscope GUI"
    print "";
    print "Parameters:";
    print "   -h  :  help"
    print "   -f  :  fake picoscope"

if __name__=="__main__":

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hf", ["help","fake"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-f", "--fake"):
            fakePicoScope = True;
        else:
            assert False, "unhandled option"
#    app = wx.PySimpleApp();
    app = wx.App(False);
    frame = MyFrame(None);
    frame.CentreOnScreen()
    frame.Show(True);
    app.MainLoop();

