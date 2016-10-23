/*
 * Controller.cpp
 *
 *  Created on: 16 Apr 2016
 *      Author: jeremy
 */

#include "MainController.h"

namespace Gui
{

	MainController::MainController(Glib::RefPtr<Gtk::Builder>& builder, std::string const& mainFolder)
	: mainFolder(mainFolder), builder(builder)
	{

		// Start drum kit
		const std::string moduleLocation(mainFolder+"/../Data/");
		drumKit = std::shared_ptr<eXaDrums>(new eXaDrums(moduleLocation.c_str()));

		// Set controllers
		metronomeController = std::unique_ptr<MetronomeController>(new MetronomeController(this->builder, this->drumKit));


		// Get all widgets
		{
			// Buttons
			builder->get_widget("AboutButton", aboutButton);
			builder->get_widget("PlayButton", playButton);
			builder->get_widget("DeleteDrumKitButton", deleteKitButton);
			builder->get_widget("RhythmCoachPrefButton", rhythmCoachPrefButton);
			builder->get_widget("AddDrumKitButton", addDrumKitButton);
			builder->get_widget("KitNameCancel", kitNameCancel);


			// Lists
			builder->get_widget("KitsList", kitsList);

			// Faders' box
			builder->get_widget("FadersSaveButton", saveFaders);
			builder->get_widget("FadersList", fadersList);

			// Entries
			builder->get_widget("KitNameEntry", kitNameEntry);

			// Dialogs
			builder->get_widget("eXaDrumsAboutDialog", aboutDialog);
			builder->get_widget("DeleteKitDialog", deleteKitDialog);
			builder->get_widget("NewKitNameDialog", newKitNameDialog);

			// Windows


		}

		// Populate Kits list
		CreateKitsList();

		// Add faders
		UpdateFaders();
		saveFaders->set_sensitive(false);


		// Load current kit
		drumKit->SelectKit(GetCurrentKitId());

		// Connect all signals
		{
			// Buttons
			aboutButton->signal_clicked().connect(sigc::mem_fun(this, &MainController::ShowAboutDialog));
			playButton->signal_clicked().connect(sigc::mem_fun(this, &MainController::PlayDrums));
			deleteKitButton->signal_clicked().connect(sigc::mem_fun(this, &MainController::DeleteKitDialog));
			saveFaders->signal_clicked().connect(sigc::mem_fun(this, &MainController::SaveFaders));
			rhythmCoachPrefButton->signal_clicked().connect(sigc::mem_fun(*metronomeController, &MetronomeController::ShowMetronomePrefs));
			addDrumKitButton->signal_clicked().connect(sigc::mem_fun(this, &MainController::ShowNewKitDialog));
			kitNameCancel->signal_clicked().connect(sigc::mem_fun(this, &MainController::HideNewKitDialog));


			// Entries
			kitNameEntry->signal_grab_focus().connect(sigc::mem_fun(this, &MainController::ShowKeyboard));

			// Kits list
			kitsList->signal_changed().connect(sigc::mem_fun(this, &MainController::ChangeKit));

			// Dialog
			aboutDialog->signal_response().connect(std::bind(sigc::mem_fun(this, &MainController::HideAboutDialog), 0));

		}

		return;
	}

	MainController::~MainController()
	{

		// Delete all pointers (dialogs and windows)
		delete deleteKitDialog;
		delete aboutDialog;
		delete newKitNameDialog;

		// Stop drum kit
		if(drumKit->IsStarted())
		{
			drumKit->Stop();
		}

		return;
	}

	// PRIVATE METHODS

	void MainController::SetInstrumentVolume(FaderPtr& fader) const
	{

		saveFaders->set_sensitive(true);
		drumKit->SetInstrumentVolume(fader->GetInstrumentId(), fader->GetValue());

		return;
	}

	void MainController::SaveFaders() const
	{

		SaveKitConfig();

		// Disable button
		saveFaders->set_sensitive(false);

		return;
	}

	void MainController::UpdateFaders()
	{

		// Remove existing faders
		std::for_each(faders.begin(), faders.end(), [](FaderPtr& fader) { fader.reset(); });
		faders.clear();

		std::vector<std::string> instNames = RetrieveInstrumentsNames();
		for(std::size_t i = 0; i < instNames.size(); i++)
		{
			std::string name(instNames[i]);
			int volume = GetInstrumentVolume(i);

			FaderPtr fader(new Fader(name, i, volume));

			faders.push_back(fader);
		}

		// Add all faders to GUI
		std::for_each(faders.cbegin(), faders.cend(), [this](FaderPtr const& f){ this->fadersList->add(*f); });

		// Connect faders signals
		for(FaderPtr& fader : faders)
		{
			fader->GetScale().signal_value_changed().connect(std::bind(sigc::mem_fun(this, &MainController::SetInstrumentVolume), std::ref(fader)));
		}

		return;
	}

	std::vector<std::string> MainController::RetrieveKitsNames() const
	{

		int numKits = drumKit->GetNumKits();

		// Get kit names
		std::vector<std::string> kitsNames(numKits);
		{
			for(int i = 0; i < numKits; i++)
			{

				// Create local array to store string given by libeXaDrums
				char kitName[128];
				int nameLength;

				// Get characters and string's length
				drumKit->GetKitNameById(i, kitName, nameLength);

				// Convert to string
				std::string name(kitName, nameLength);

				kitsNames[i] = name;
			}
		}

		return kitsNames;
	}

	void MainController::CreateKitsList()
	{

		// Retrieve kits names
		std::vector<std::string> kitsNames = RetrieveKitsNames();

		// Add kits to the list
		for(std::size_t i = 0; i < kitsNames.size(); i++)
		{
			kitsList->insert(i, kitsNames[i]);
		}
		// Set default kit
		kitsList->set_active(0);

		return;
	}

	void MainController::DeleteKit(const int& id)
	{

		int numKits = drumKit->GetNumKits();

		// Can't delete if only one kit
		if(numKits > 1)
		{
			// Stop module if started
			if(drumKit->IsStarted())
			{
				PlayDrums();
			}

			// Deselect kit
			int activeKit;

			if(id == 0)
			{
				activeKit = 1;
			}
			else
			{
				activeKit = id - 1;
			}


			kitsList->set_active(activeKit);

			// Delete kit
			drumKit->DeleteKit(id);

			// Remove kit from list
			kitsList->remove_text(id);

			// Set new kit
			ChangeKit();

		}


		return;
	}


	std::vector<std::string> MainController::RetrieveInstrumentsNames() const
	{

		int numInstruments = drumKit->GetNumInstruments();

		// Get instruments names
		std::vector<std::string> instNames(numInstruments);
		{
			for(int i = 0; i < numInstruments; i++)
			{

				// Create local array to store string given by libeXaDrums
				char instName[128];
				int nameLength;

				// Get characters and string's length
				drumKit->GetInstrumentName(i, instName, nameLength);

				// Convert to string
				std::string name(instName, nameLength);

				instNames[i] = name;
			}
		}

		return instNames;
	}


	int MainController::GetCurrentKitId() const
	{

		int currentKitId = kitsList->get_active_row_number();

		return currentKitId;
	}


	void MainController::ShowAboutDialog()
	{
		aboutDialog->show();
		return;
	}

	void MainController::HideAboutDialog(int responseId)
	{
		aboutDialog->hide();
		return;
	}

	void MainController::ShowKeyboard()
	{
		HideKeyboard();
		std::system("onboard --size=800x160 -x 0 -y 280 &");
		return;
	}

	void MainController::HideKeyboard()
	{

		std::system("killall onboard");
		return;
	}


	void MainController::ShowNewKitDialog()
	{

		newKitNameDialog->show();
		ShowKeyboard();

		return;
	}

	void MainController::HideNewKitDialog()
	{

		HideKeyboard();
		newKitNameDialog->hide();
		return;
	}


	void MainController::PlayDrums()
	{

		if(drumKit->IsStarted())
		{
			this->playButton->set_property("label", Gtk::StockID("gtk-media-play"));
			drumKit->Stop();
		}
		else
		{
			this->playButton->set_property("label", Gtk::StockID("gtk-media-stop"));
			drumKit->Start();
		}

		return;
	}

	void MainController::ChangeKit()
	{

		bool started = drumKit->IsStarted();

		// Stop module if started
		if(started)
		{
			drumKit->Stop();
		}


		// Load new kit
		drumKit->SelectKit(GetCurrentKitId());

		// Update faders
		this->UpdateFaders();
		saveFaders->set_sensitive(false);

		// Restart module if needed
		if(started)
		{
			drumKit->Start();
		}

		return;
	}

	void MainController::DeleteKitDialog()
	{

		// Get answer
		int answer = deleteKitDialog->run();

		// Check answer
		switch (answer)
		{
			case Gtk::RESPONSE_CANCEL: break;
			case Gtk::RESPONSE_OK: this->DeleteKit(GetCurrentKitId()); break;
			default: break;
		}

		// Close dialog
		deleteKitDialog->hide();

		return;
	}

} /* namespace Gui */
