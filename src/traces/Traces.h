#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

typedef tr1::shared_ptr<ofxXmlSettings> ofxXmlSettings_ptr;

namespace guardacaso {

    class Traces {
		
		public:
		
            static Traces&	get();
            void			setup();

            void			update();
            void			reloadServer(ofxXmlSettings_ptr xml);
            void            saveServer();
            string          clientId();
            string          historyDir();

            void simulateGroup(string group_dir);
            void simulateGroups();

		protected:

            Traces();
            ~Traces();
			
        private:

            void			setupServer();

            string          xml_server;
            string          client_id;
            string          history_dir;
			
	};
	
}
