/*
 * Controller.h
 *
 *  Created on: 16 Apr 2016
 *      Author: jeremy
 */

#ifndef SOURCE_CONTROLLER_CONTROLLER_H_
#define SOURCE_CONTROLLER_CONTROLLER_H_

#include <Source/Api/eXaDrums.h>

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/comboboxtext.h>

#include <string>
#include <memory>
#include <functional>

using namespace eXaDrumsApi;

namespace Gui
{

	class Controller
	{

	public:

		Controller(Glib::RefPtr<Gtk::Builder>& builder, std::string const& mainFolder);
		virtual ~Controller();

	private:

		void ShowAboutDialog();
		void HideAboutDialog(int responseId);
		void PlayDrums();
		void ChangeKit();

		std::string GetCurrentKitName() const;
		std::string GetKitNameById(int id) const;

		std::string mainFolder;

		std::unique_ptr<eXaDrums> drumKit;
		Glib::RefPtr<Gtk::Builder> builder;

		// Buttons
		Gtk::Button* aboutButton;
		Gtk::Button* playButton;

		// Comboboxes
		Gtk::ComboBoxText* kitsList;

		// Dialogs
		Gtk::AboutDialog* aboutDialog;

	};

} /* namespace Gui */

#endif /* SOURCE_CONTROLLER_CONTROLLER_H_ */
