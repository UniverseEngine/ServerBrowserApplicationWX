#include "pch.hpp"

#include "dialogs/AboutDialog.hpp"

#include "MyFrame.hpp"

enum
{
    DIALOG_OK = 1
};

AboutDialog::AboutDialog(const wxString& title)
    : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    {
        wxBoxSizer* aboutSizer = new wxBoxSizer(wxVERTICAL);

        aboutSizer->Add(new wxStaticText(this, wxID_ANY, "ServerBrowserApplication", wxDefaultPosition, wxDefaultSize));
        aboutSizer->Add(new wxStaticText(this, wxID_ANY, " ", wxDefaultPosition, wxDefaultSize));
        aboutSizer->Add(new wxStaticText(this, wxID_ANY, "UniverseDevelopmentFramework © 2023", wxDefaultPosition, wxDefaultSize));
        aboutSizer->Add(new wxStaticText(this, wxID_ANY, "Developers: lucx, perikiyoxd", wxDefaultPosition, wxDefaultSize));
        aboutSizer->Add(new wxStaticText(this, wxID_ANY, "Website: http://lc-mp.org", wxDefaultPosition, wxDefaultSize));

        sizer->Add(aboutSizer, 5, wxALL | wxEXPAND, 5);
    }

    /* buttons */
    {
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* closeButton = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(70, 24));
        closeButton->Bind(wxEVT_BUTTON, &AboutDialog::OnOK, this);

        buttonSizer->Add(closeButton, 1);

        sizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
    }

    SetSizerAndFit(sizer);

    SetMinSize(wxSize(1000, 600));

    Bind(wxEVT_CLOSE_WINDOW, &AboutDialog::OnClose, this);
}

void AboutDialog::OnClose(wxCloseEvent&)
{
    EndModal(wxID_OK);
}

void AboutDialog::OnOK(wxCommandEvent&)
{
    EndModal(wxID_OK);
}