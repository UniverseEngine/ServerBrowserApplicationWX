#include "pch.hpp"

#include "dialogs/SettingsDialog.hpp"

#include "MyFrame.hpp"

#include "Browser.hpp"

enum
{
    DIALOG_OK = 1,
    DIALOG_CLOSE,
    DIALOG_BROWSE,
    DIALOG_WINDOWED
};

SettingsDialog::SettingsDialog(const wxString& title)
    : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxNotebook(this, wxID_ANY);

    mainSizer->Add(m_notebook);

    /* General */
    {
        auto panel = new wxPanel(m_notebook);
        m_notebook->AddPage(panel, "General", true);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        /* nickname */
        auto nickname_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Nickname");
        {
            m_nicknameInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(240, 24));
            m_nicknameInput->SetValue(gBrowser->m_settings.nickname);

            nickname_sizer->Add(m_nicknameInput, 1, wxALL | wxEXPAND, 5);
        }

        sizer->Add(nickname_sizer, 0, wxALL | wxEXPAND, 5);

        /* masterlist */
        auto masterlist_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Masterlist");
        {
            m_masterlistInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(240, 24));
            m_masterlistInput->SetValue(gBrowser->m_settings.masterlist);

            masterlist_sizer->Add(m_masterlistInput, 1, wxALL | wxEXPAND, 5);
        }

        sizer->Add(masterlist_sizer, 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer);
    }

    /* GTA:III Settings */
    {
        auto panel = new wxPanel(m_notebook);
        m_notebook->AddPage(panel, "GTA:III", true);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        /* game path */
        auto gamePath_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Install Path");
        {
            m_gamePath = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(280, 24));
            m_gamePath->SetValue(gBrowser->m_settings.gamePath.string());

            wxButton* browseButton = new wxButton(panel, DIALOG_BROWSE, "Browse", wxDefaultPosition, wxSize(280, 24));
            browseButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnBrowse, this);

            gamePath_sizer->Add(m_gamePath, 1, wxALL | wxEXPAND, 5);
            gamePath_sizer->Add(browseButton, 1, wxALL | wxEXPAND, 5);
        }

        sizer->Add(gamePath_sizer, 0, wxALL | wxEXPAND, 5);

        /* settings */
        auto gameSettings_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Game Settings");
        {
            m_windowedCheckbox = new wxCheckBox(panel, DIALOG_WINDOWED, "Windowed");
            m_windowedCheckbox->SetValue(gBrowser->m_settings.windowed);
            gameSettings_sizer->Add(m_windowedCheckbox, 1);

            gameSettings_sizer->SetMinSize(150, 0);
        }

        sizer->Add(gameSettings_sizer, 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer);
    }

    /* Developer Settings */
    {
        auto panel = new wxPanel(m_notebook);
        m_notebook->AddPage(panel, "Developer", true);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        /* developer settings */
        auto developerSettings_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Developer Settings");
        {
            /* checkbox: Show console */
            m_showConsoleCheckbox = new wxCheckBox(panel, DIALOG_WINDOWED, "Show console");
            m_showConsoleCheckbox->SetValue(gBrowser->m_settings.showConsole);
            developerSettings_sizer->Add(m_showConsoleCheckbox, 1);

            developerSettings_sizer->SetMinSize(150, 0);
        }

        sizer->Add(developerSettings_sizer, 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer);
    }

    m_notebook->SetSelection(0);

    /* buttons */
    {
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxButton* okButton = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(70, 24));
        okButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnOK, this);

        buttonSizer->Add(okButton, 1);

        wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(70, 24));
        cancelButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnCancel, this);

        buttonSizer->Add(cancelButton, 1, wxLEFT, 5);

        mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxTOP | wxBOTTOM, 10);
    }

    SetSizerAndFit(mainSizer);

    Bind(wxEVT_CLOSE_WINDOW, &SettingsDialog::OnClose, this);
    Bind(wxEVT_CHECKBOX, &SettingsDialog::OnCheckbox, this);
}

void SettingsDialog::OnClose(wxCloseEvent&)
{
    EndModal(wxID_CANCEL);
}

void SettingsDialog::OnCheckbox(wxCommandEvent& event)
{
    auto& settings = gBrowser->m_settings;
    switch (event.GetId())
    {
    case DIALOG_WINDOWED:
        settings.windowed = (bool)event.GetInt();
        break;
    }
}

void SettingsDialog::OnCancel(wxCommandEvent&)
{
    EndModal(wxID_CANCEL);
}

void SettingsDialog::OnBrowse(wxCommandEvent&)
{
    wxFileDialog browseDialog(this, "Open GTA3 EXE", "", "", "GTA3 EXE (gta3.exe)|*.exe", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (browseDialog.ShowModal() == wxID_CANCEL)
        return;
    gBrowser->m_settings.gamePath = Path(browseDialog.GetPath().ToStdString());

    m_gamePath->SetValue(gBrowser->m_settings.gamePath.string());
}

void SettingsDialog::OnOK(wxCommandEvent& event)
{
    gBrowser->m_settings.nickname    = m_nicknameInput->GetValue();
    gBrowser->m_settings.gamePath    = Path(m_gamePath->GetValue().ToStdString());
    gBrowser->m_settings.masterlist  = m_masterlistInput->GetValue();
    gBrowser->m_settings.windowed    = m_windowedCheckbox->GetValue();
    gBrowser->m_settings.showConsole = m_showConsoleCheckbox->GetValue();

    gBrowser->SaveSettings();

    EndModal(wxID_CANCEL);
}