#include <wx/wx.h>

class AddServerDialog : public wxDialog {
public:
    AddServerDialog(const wxString& title);

private:
    wxTextCtrl* m_input;

    void OnClose(wxCloseEvent& event);
    void OnCancel(wxCommandEvent&);
    void OnOK(wxCommandEvent&);
};