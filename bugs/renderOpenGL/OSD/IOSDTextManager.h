#pragma once

namespace Render
{
	class IOSDTextManager abstract
	{
	public:
		virtual ~IOSDTextManager() { }

		// add a line of text to the OSD (the line will be positioned below all existing lines).
		// returns the ID of the line (use this to remove it)
		virtual int addLine(wstring line)=0;
		
		// add a line of text to the OSD in a specified color
		// returns the ID of the line (use this to remove it)
		virtual int addLineColor(wstring line, DWORD color)=0;

		// changes the text of an existing line
		virtual void updateLine(int lineID, wstring newText)=0;

		// removes a line from the OSD (use the id of the line received when adding it)
		virtual void removeLine(int lineID)=0;
	};
}