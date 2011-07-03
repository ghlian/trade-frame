/************************************************************************
 * Copyright(c) 2011, One Unified. All rights reserved.                 *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

// Hedge.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <wx/wx.h>
#include <wx/splitter.h>
//#include <wx/treectrl.h>

#include "VuPortfolios.h"
#include "VuPositions.h"
#include "VuOrders.h"
#include "VuExecutions.h"

class MyApp : public wxApp {
   virtual bool OnInit();
   virtual int OnExit();
};
 
IMPLEMENT_APP(MyApp)

// Implements MyApp& wxGetApp()
DECLARE_APP(MyApp)
 
class MyFrame : public wxFrame {
public:
   MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
protected:
   // Do we really need to expose the implementation detail? I guess not.
   void OnQuit(wxCommandEvent& event);
   void OnAbout(wxCommandEvent& event);
private:
   enum { ID_Quit=wxID_HIGHEST + 1, ID_About, 
     ID_ConnectIB, ID_ConnectIQFeed, ID_ConnectSim,
     ID_DisConnectIB, ID_DisConnectIQFeed, ID_DisConnectSim,
     ID_AddPortfolio
   };
};
 
bool MyApp::OnInit() {
   wxFrame *frame = new MyFrame("Hedge", wxPoint(50,50), wxSize(600,900));
     frame->Show(TRUE);
     SetTopWindow(frame);
     return TRUE;
}

int MyApp::OnExit() {
  return 0;
}
 
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL, -1, title, pos, size)
{
  // create menubar
  wxMenuBar* menuBar = new wxMenuBar;
  // create menu
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(ID_About, "&About...\tF1");
  menuFile->AppendSeparator();
  menuFile->Append(ID_Quit, "E&xit\tAlt-X");
  menuBar->Append(menuFile, "&File");

  wxMenu* menuConnect = new wxMenu;
  menuConnect->Append( ID_ConnectIB, "&Interactive Brokers" );
  menuConnect->Append( ID_ConnectIQFeed, "IQ&Feed" );
  menuConnect->Append( ID_ConnectSim, "&Simulation" );
  menuBar->Append( menuConnect, "&Connect" );

  wxMenu* menuDisConnect = new wxMenu;
  menuDisConnect->Append( ID_DisConnectIB, "&Interactive Brokers" );
  menuDisConnect->Append( ID_DisConnectIQFeed, "IQ&Feed" );
  menuDisConnect->Append( ID_DisConnectSim, "&Simulation" );
  menuBar->Append( menuDisConnect, "&Disconnect" );

  wxMenu* menuActivities = new wxMenu;
  menuActivities->Append( ID_AddPortfolio, "Add P&ortfolio" );
  menuBar->Append( menuActivities, "&Activities" );

  // set frame menubar
  SetMenuBar(menuBar);
 
  Bind( wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnQuit, this, ID_Quit );
  Bind( wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnAbout, this, ID_About );
 
  CreateStatusBar();
  SetStatusText("Hedge");

  // Portfolio View sized with Positions, Orders, Executions
  wxBoxSizer* pSizerForFrame = new wxBoxSizer( wxVERTICAL );
  SetSizer( pSizerForFrame );

  wxSplitterWindow* pSplitPortfolioAndRemainder = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER );
  pSplitPortfolioAndRemainder->SetMinimumPaneSize( 20 );

  pSizerForFrame->Add( pSplitPortfolioAndRemainder, 1, wxEXPAND | wxALL, 5 );

  VuPortfolios* pDVPortfolios = new VuPortfolios( pSplitPortfolioAndRemainder, wxID_ANY );

  wxPanel* pPanelForPositionsOrdersExecutions = new wxPanel( pSplitPortfolioAndRemainder, wxID_ANY );

  pSplitPortfolioAndRemainder->Initialize( pDVPortfolios );
  pSplitPortfolioAndRemainder->SplitHorizontally( pDVPortfolios, pPanelForPositionsOrdersExecutions );

  pPanelForPositionsOrdersExecutions->Show( true );

  // Position view sized with Orders, Executions

  wxBoxSizer* pSizerForPositionOrderExecution = new wxBoxSizer( wxVERTICAL );
  pPanelForPositionsOrdersExecutions->SetSizer( pSizerForPositionOrderExecution );

  wxSplitterWindow* pSplitForPositionAndOrderExecution = new wxSplitterWindow( pPanelForPositionsOrdersExecutions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER );
  pSplitForPositionAndOrderExecution->SetMinimumPaneSize( 20 );

  pSizerForPositionOrderExecution->Add( pSplitForPositionAndOrderExecution, 1, wxEXPAND | wxALL, 5 );

  VuPositions* pDVPositions = new VuPositions( pSplitForPositionAndOrderExecution, wxID_ANY );

  wxPanel* pPanelForOrdersExecutions = new wxPanel( pSplitForPositionAndOrderExecution, wxID_ANY );

  pSplitForPositionAndOrderExecution->Initialize( pDVPositions );
  pSplitForPositionAndOrderExecution->SplitHorizontally( pDVPositions, pPanelForOrdersExecutions );

  pPanelForOrdersExecutions->Show( true );

  // Order view sized with Executions

  wxBoxSizer* pSizerForOrderAndExecution = new wxBoxSizer( wxVERTICAL );
  pPanelForOrdersExecutions->SetSizer( pSizerForOrderAndExecution );

  wxSplitterWindow* pSplitForOrderAndExecution = new wxSplitterWindow( pPanelForOrdersExecutions, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER );
  pSplitForOrderAndExecution->SetMinimumPaneSize( 20 );

  pSizerForOrderAndExecution->Add( pSplitForOrderAndExecution, 1, wxEXPAND | wxALL, 5 );

  VuOrders* pDVOrders = new VuOrders( pSplitForOrderAndExecution, wxID_ANY );

  VuExecutions* pDVExecutions = new VuExecutions( pSplitForOrderAndExecution, wxID_ANY );

  pSplitForOrderAndExecution->Initialize( pDVOrders );
  pSplitForOrderAndExecution->SplitHorizontally( pDVOrders, pDVExecutions );

/*
  wxTreeCtrl* tree = new wxTreeCtrl( split, wxID_ANY );
  wxTreeItemId treeRoot = tree->AddRoot( wxT( "Hedge" ) );
  wxTreeItemId itemAccounts = tree->AppendItem( treeRoot, wxT( "Accounts" ) );
  wxTreeItemId itemPortfolios = tree->AppendItem( treeRoot, wxT( "Portfolios" ) );
  wxTreeItemId itemInstruments = tree->AppendItem( treeRoot, wxT( "Instruments" ) );
  wxTreeItemId itemExchanges = tree->AppendItem( treeRoot, wxT( "Exchanges" ) );
*/
//  wxPanel *panelVarious = new wxPanel( split, wxID_ANY );

//  tree->Show( true );
//  panelVarious->Show( true );

//  wxStaticBox *st = new wxStaticBox(panel, -1, wxT("Colors"), wxPoint(5, 5), wxSize(240, 150));

//  wxRadioButton *rb = new wxRadioButton(panel, -1, wxT("256 Colors"), wxPoint(15, 30), wxDefaultSize, wxRB_GROUP);
//  wxRadioButton *rb1 = new wxRadioButton(panel, -1, wxT("16 Colors"), wxPoint(15, 55));
//  wxRadioButton *rb2 = new wxRadioButton(panel, -1, wxT("2 Colors"), wxPoint(15, 80));
//  wxRadioButton *rb3 = new wxRadioButton(panel, -1, wxT("Custom"), wxPoint(15, 105));

//  wxTextCtrl *tc = new wxTextCtrl(panel, -1, wxT(""), wxPoint(95, 105));

  //wxButton *okButton = new wxButton(panel, -1, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
  //wxButton *closeButton = new wxButton(panel, -1, wxT("Close"), wxDefaultPosition, wxSize(70, 30));

//  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
//  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

//  hbox->Add(okButton, 1);
//  hbox->Add(closeButton, 1, wxLEFT, 5);

//  vbox->Add( split, 1, wxEXPAND|wxALL, 5 );
//  vbox->Add(panel, 1);
//  vbox->Add(hbox, 1, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

  Centre();

}
 
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
   Close(TRUE);
}
 
void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
   wxMessageBox( "Hedge\nCopyright (2011) One Unified\nWritten By: Raymond Burkholder\ninfo@oneunified.net", "TradeFrame FrameWork",
        wxOK|wxICON_INFORMATION, this);
}
