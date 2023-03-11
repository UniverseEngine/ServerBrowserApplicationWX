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
    : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    /* input */
    {
        wxBoxSizer* aboutSizer = new wxBoxSizer(wxVERTICAL);

        aboutSizer->Add(m_input = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250, 24)));

        sizer->Add(aboutSizer, wxSizerFlags().Expand().Proportion(5).Border(wxALL, 5));
    }

    /* buttons */
    {
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* okButton = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(70, 24));
        okButton->Bind(wxEVT_BUTTON, &AddServerDialog::OnOK, this);

        buttonSizer->Add(okButton, 1);

        wxButton* cancelButton = new wxButton(this, wxID_OK, "Cancel", wxDefaultPosition, wxSize(70, 24));
        cancelButton->Bind(wxEVT_BUTTON, &AddServerDialog::OnCancel, this);

        buttonSizer->Add(cancelButton, 1);

        sizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
    }

    SetSizerAndFit(sizer);

    Centre();

    Bind(wxEVT_CLOSE_WINDOW, &AddServerDialog::OnClose, this);
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

    gBrowser->AddToFavorites(ServerHost(text));

    EndModal(wxID_CANCEL);
}