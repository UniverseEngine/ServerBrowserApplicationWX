#include "pch.hpp"

#include "dialogs/AddServerDialog.hpp"

#include "MyFrame.hpp"

#include "Browser.hpp"

enum
{
    DIALOG_OK = 1,
    DIALOG_CLOSE
};

AddServerDialog::AddServerDialog(const wxString& title)
    : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 110))
{
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    m_input = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(5, 5), wxSize(220, 24));

    wxButton* okButton     = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(70, 24));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(70, 24));

    hbox->Add(okButton, 1);
    hbox->Add(cancelButton, 1, wxLEFT, 5);

    vbox->Add(panel, 1);
    vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

    SetSizer(vbox);

    Centre();

    Bind(wxEVT_CLOSE_WINDOW, &AddServerDialog::OnClose, this);

    okButton->Bind(wxEVT_BUTTON, &AddServerDialog::OnOK, this);
    cancelButton->Bind(wxEVT_BUTTON, &AddServerDialog::OnCancel, this);
}

void AddServerDialog::OnClose(wxCloseEvent&)
{
    EndModal(wxID_CANCEL);
}

void AddServerDialog::OnCancel(wxCommandEvent&)
{
    EndModal(wxID_CANCEL);
}

void AddServerDialog::OnOK(wxCommandEvent& event)
{
    String text = m_input->GetLineText(0).ToStdString();

    String   ip   = text.substr(0, text.find(":"));
    uint16_t port = std::atoi(text.substr(text.find(":") + 1).c_str());

    gBrowser->AddToFavorites(ip, port);

    EndModal(wxID_CANCEL);
}