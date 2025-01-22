#include <wx/wx.h>
#include <memory>

class wxBoxSizer;
class wxNotebook;
class wxTextCtrl;
class wxCheckBox;

class SettingsDialog : public wxDialog {
public:
    explicit SettingsDialog(const wxString& title);

private:
    std::unique_ptr<wxBoxSizer> m_mainSizer;
    std::unique_ptr<wxNotebook> m_notebook;
    std::unique_ptr<wxTextCtrl> m_nicknameInput;
    std::unique_ptr<wxTextCtrl> m_masterlistInput;
    std::unique_ptr<wxTextCtrl> m_gamePath;
    std::unique_ptr<wxCheckBox> m_windowedCheckbox;
    std::unique_ptr<wxCheckBox> m_showConsoleCheckbox;

    void OnClose(wxCloseEvent&);
    void OnCheckbox(wxCommandEvent&);
    void OnCancel(wxCommandEvent&);
    void OnBrowse(wxCommandEvent&);
    void OnOK(wxCommandEvent&);
};
