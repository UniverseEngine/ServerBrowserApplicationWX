#include <wx/wx.h>

class AboutDialog : public wxDialog {
public:
    AboutDialog(const wxString& title);

private:
    void OnClose(wxCloseEvent& event);
    void OnOK(wxCommandEvent&);
};