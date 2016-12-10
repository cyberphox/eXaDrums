/*
 * SoundTypeAndPath.h
 *
 *  Created on: 6 Dec 2016
 *      Author: jeremy
 */

#ifndef SOURCE_WIDGETS_SOUNDTYPEANDPATH_H_
#define SOURCE_WIDGETS_SOUNDTYPEANDPATH_H_

#include <gtkmm/grid.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/label.h>

#include <vector>
#include <string>
#include <memory>

namespace Widgets
{

	class SoundTypeAndPath : public Gtk::Grid
	{

	public:

		SoundTypeAndPath(const std::vector<std::string>& types, const std::vector<std::string>& sounds);
		virtual ~SoundTypeAndPath();

	private:

		Gtk::Label soundTypeLabel;
		Gtk::Label soundLabel;
		Gtk::ComboBoxText typesList;
		Gtk::ComboBoxText soundsList;

	};

	typedef std::shared_ptr<SoundTypeAndPath> SoundTypeAndPathPtr;

} /* namespace Widgets */

#endif /* SOURCE_WIDGETS_SOUNDTYPEANDPATH_H_ */
