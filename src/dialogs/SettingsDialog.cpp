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
    : wxDialog(nullptr, wxID_ANY, title, wxDefaultPosition)
{
    m_mainSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    m_notebook = std::make_unique<wxNotebook>(this, wxID_ANY);
    m_mainSizer->Add(m_notebook.get());

    /* General */
    {
        auto panel = std::make_unique<wxPanel>(m_notebook.get());
        m_notebook->AddPage(panel.get(), "General", true);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        /* nickname */
        auto nicknameSizer = std::make_unique<wxStaticBoxSizer>(wxVERTICAL, panel.get(), "Nickname");
        {
            m_nicknameInput = std::make_unique<wxTextCtrl>(panel.get(), wxID_ANY, "", wxDefaultPosition, wxSize(240, 24));
            m_nicknameInput->SetValue(gBrowser->m_settings.nickname);

            nicknameSizer->Add(m_nicknameInput.get(), 1, wxALL | wxEXPAND, 5);
        }
        sizer->Add(nicknameSizer.get(), 0, wxALL | wxEXPAND, 5);

        /* masterlist */
        auto masterlistSizer = std::make_unique<wxStaticBoxSizer>(wxVERTICAL, panel.get(), "Masterlist");
        {
            m_masterlistInput = std::make_unique<wxTextCtrl>(panel.get(), wxID_ANY, "", wxDefaultPosition, wxSize(240, 24));
            m_masterlistInput->SetValue(gBrowser->m_settings.masterlist);

            masterlistSizer->Add(m_masterlistInput.get(), 1, wxALL | wxEXPAND, 5);
        }
        sizer->Add(masterlistSizer.get(), 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer.get());
    }

    /* GTA:III Settings */
    {
        auto panel = std::make_unique<wxPanel>(m_notebook.get());
        m_notebook->AddPage(panel.get(), "GTA:III", true);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        /* game path */
        auto gamePathSizer = std::make_unique<wxStaticBoxSizer>(wxVERTICAL, panel.get(), "Install Path");
        {
            m_gamePath = std::make_unique<wxTextCtrl>(panel.get(), wxID_ANY, "", wxDefaultPosition, wxSize(280, 24));
            m_gamePath->SetValue(gBrowser->m_settings.gamePath.string());

            auto browseButton = std::make_unique<wxButton>(panel.get(), DIALOG_BROWSE, "Browse", wxDefaultPosition, wxSize(280, 24));
            browseButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnBrowse, this);

            gamePathSizer->Add(m_gamePath.get(), 1, wxALL | wxEXPAND, 5);
            gamePathSizer->Add(browseButton.get(), 1, wxALL | wxEXPAND, 5);
        }
        sizer->Add(gamePathSizer.get(), 0, wxALL | wxEXPAND, 5);

        /* settings */
        auto gameSettingsSizer = std::make_unique<wxStaticBoxSizer>(wxVERTICAL, panel.get(), "Game Settings");
        {
            m_windowedCheckbox = std::make_unique<wxCheckBox>(panel.get(), DIALOG_WINDOWED, "Windowed");
            m_windowedCheckbox->SetValue(gBrowser->m_settings.windowed);
            gameSettingsSizer->Add(m_windowedCheckbox.get(), 1);

            gameSettingsSizer->SetMinSize(150, 0);
        }
        sizer->Add(gameSettingsSizer.get(), 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer.get());
    }

    /* Developer Settings */
    {
        auto panel = std::make_unique<wxPanel>(m_notebook.get());
        m_notebook->AddPage(panel.get(), "Developer", true);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        /* developer settings */
        auto developerSettingsSizer = std::make_unique<wxStaticBoxSizer>(wxVERTICAL, panel.get(), "Developer Settings");
        {
            /* checkbox: Show console */
            m_showConsoleCheckbox = std::make_unique<wxCheckBox>(panel.get(), DIALOG_WINDOWED, "Show console");
            m_showConsoleCheckbox->SetValue(gBrowser->m_settings.showConsole);
            developerSettingsSizer->Add(m_showConsoleCheckbox.get(), 1);

            developerSettingsSizer->SetMinSize(150, 0);
        }
        sizer->Add(developerSettingsSizer.get(), 0, wxALL | wxEXPAND, 5);

        panel->SetSizerAndFit(sizer.get());
    }

    m_notebook->SetSelection(0);

    /* buttons */
    {
        auto okButton = std::make_unique<wxButton>(this, wxID_OK, "OK", wxDefaultPosition, wxSize(70, 24));
        okButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnOK, this);

        auto cancelButton = std::make_unique<wxButton>(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(70, 24));
        cancelButton->Bind(wxEVT_BUTTON, &SettingsDialog::OnCancel, this);

        auto buttonSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        buttonSizer->Add(okButton.get(), 1);
        buttonSizer->Add(cancelButton.get(), 1, wxLEFT, 5);

        m_mainSizer->Add(buttonSizer.get(), 0, wxALIGN_RIGHT | wxTOP | wxBOTTOM, 10);
    }

    SetSizerAndFit(m_mainSizer.get());

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
    if (event.GetId() == DIALOG_WINDOWED) {
        settings.windowed = static_cast<bool>(event.GetInt());
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
    gBrowser->m_settings.gamePath = std::filesystem::path(browseDialog.GetPath().ToStdString());

    m_gamePath->SetValue(gBrowser->m_settings.gamePath.string());
}

void SettingsDialog::OnOK(wxCommandEvent&)
{
    gBrowser->m_settings.nickname    = m_nicknameInput->GetValue();
    gBrowser->m_settings.gamePath    = std::filesystem::path(m_gamePath->GetValue().ToStdString());
    gBrowser->m_settings.masterlist  = m_masterlistInput->GetValue();
    gBrowser->m_settings.windowed    = m_windowedCheckbox->GetValue();
    gBrowser->m_settings.showConsole = m_showConsoleCheckbox->GetValue();

    gBrowser->SaveSettings();

    EndModal(wxID_CANCEL);
}
