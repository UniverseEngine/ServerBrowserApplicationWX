#include "pch.hpp"

#include "dialogs/AboutDialog.hpp"

#include "MyFrame.hpp"

#include <wx/gbsizer.h>

enum
{
    DIALOG_OK = 1
};

AboutDialog::AboutDialog(const wxString& title)
    : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    {
        wxBoxSizer* aboutInfoSizer = new wxBoxSizer(wxVERTICAL);

        wxGridBagSizer* gbs = new wxGridBagSizer(0, 15);
        {
            /* bitmap */
            {
                auto bitmap = wxBitmap();
                bitmap.CopyFromIcon(wxICON(IDI_APPICON));
                auto staticBitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

                gbs->Add(staticBitmap, wxGBPosition(0, 0));
            }

            gbs->Add(new wxStaticText(this, wxID_ANY, "Server Browser", wxDefaultPosition, wxDefaultSize), wxGBPosition(0, 1));
            gbs->Add(new wxStaticText(this, wxID_ANY, "Copyright \xa9 UniverseDevelopmentFramework 2023", wxDefaultPosition, wxDefaultSize), wxGBPosition(1, 1));
            gbs->Add(new wxStaticText(this, wxID_ANY, "Developers: lucx, perikiyoxd", wxDefaultPosition, wxDefaultSize), wxGBPosition(2, 1));
            gbs->Add(new wxStaticText(this, wxID_ANY, "Website: https://lc-mp.org", wxDefaultPosition, wxDefaultSize), wxGBPosition(3, 1));
        }

        aboutInfoSizer->Add(gbs, 1, wxALL | wxEXPAND, 5);

        sizer->Add(aboutInfoSizer, 5, wxALL | wxEXPAND, 5);
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