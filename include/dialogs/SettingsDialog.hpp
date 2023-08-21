#include <wx/wx.h>

#include <wx/notebook.h>

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(const wxString& title);

private:
    wxNotebook* m_notebook;
    wxTextCtrl* m_nicknameInput;
    wxTextCtrl* m_masterlistInput;
    wxTextCtrl* m_gamePath;
    wxCheckBox* m_windowedCheckbox;
    wxCheckBox* m_showConsoleCheckbox;

    void OnClose(wxCloseEvent&);
    void OnCheckbox(wxCommandEvent&);
    void OnCancel(wxCommandEvent&);
    void OnBrowse(wxCommandEvent&);
    void OnOK(wxCommandEvent&);
};