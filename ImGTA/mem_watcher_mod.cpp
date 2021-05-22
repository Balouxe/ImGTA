#include "mem_watcher_mod.h"
#include "imgui.h"
#include "script.h"
#include "main.h"
#include "natives.h"
#include "utils.h"
#include "imgui_extras.h"
#include "global_id.h"

#include <vector>
#include <stdio.h>
#include <inttypes.h>
#include <mutex>
#include <algorithm>
#include <bitset>

const char *watchTypeNames[] = { "Int", "Float", "String", "Vector", "Bitfield" };

void MemWatcherMod::Load()
{

}

void MemWatcherMod::Unload()
{

}

void MemWatcherMod::Think()
{
	if (m_watches.size() > 0)
	{
		std::lock_guard<std::mutex> lock(m_watchesMutex);

		char buf[128] = "";
		float yOff = m_inGameOffsetY;

		m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
		// Check if script is still running
		if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
			m_scriptRunning = true;
		else
			m_scriptRunning = false;

		sprintf_s(buf, "Script running: %s", BoolToStr(m_scriptRunning));
		DrawTextToScreen(buf, m_inGameOffsetX, yOff, m_inGameFontSize, eFont::FontChaletLondon);
		yOff += 0.02f;

		for (auto &w : m_watches)
		{
			if (m_askForThreadId)
			{
				// Re-check if script is still running
				if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
				{
					m_scriptRunning = true;
					w.UpdateValue(m_scriptHash);
				}
				else
					m_scriptRunning = false;
			}
			else
				w.UpdateValue();
			if (w.m_drawInGame && m_showInGame)
			{
				sprintf_s(buf, "%d (0x%x): %s", w.m_addressIndex, w.m_addressIndex, w.m_value.c_str());
				DrawTextToScreen(buf, m_inGameOffsetX, yOff, m_inGameFontSize, eFont::FontChaletLondon);
				yOff += 0.02f;
			}
		}
	}
}

void MemWatcherMod::ShowAddAddress(bool isGlobal)
{
	if (m_inputHexIndex)
		ImGui::InputInt("Hex Index##AddAddress", &m_inputAddressIndex, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	else
		ImGui::InputInt("Decimal Index##AddAddress", &m_inputAddressIndex, 1, 100);
	ImGui::Combo("Type##AddAddress", &m_inputType, watchTypeNames, IM_ARRAYSIZE(watchTypeNames));
	if (!isGlobal)
	{
		if (ImGui::InputText("Script Name##AddAddress", m_scriptNameBuf, sizeof(m_scriptNameBuf)))
		{
			m_scriptName = std::string(m_scriptNameBuf);
			RunOnNativeThread([&]
			{
				m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
				if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
					m_scriptRunning = true;
				else
					m_scriptRunning = false;
			});
		}
	}

	if (ImGui::InputText("Info##AddAddress", m_watchInfoBuf, sizeof(m_watchInfoBuf)))
		m_watchInfo = std::string(m_watchInfoBuf);

	if (isGlobal || (!isGlobal && m_scriptRunning))
	{
		if (ImGui::Button("Add##AddAddress"))
		{
			if (isGlobal)
			{
				if (getGlobalPtr(m_inputAddressIndex) == nullptr)
					m_addressUnavailable = true;
			}
			else
			{
				if (GetThreadAddress(m_inputAddressIndex, m_scriptHash) == nullptr)
					m_addressUnavailable = true;
			}

			m_addressUnavailable = false;

			std::lock_guard<std::mutex> lock(m_watchesMutex);
			if (isGlobal)
				m_watches.push_back(WatchEntry(m_inputAddressIndex, (WatchType)m_inputType, "Global", 0, m_watchInfo));
			else
				m_watches.push_back(WatchEntry(m_inputAddressIndex, (WatchType)m_inputType, m_scriptName, m_scriptHash, m_watchInfo));
		}

		if (m_addressUnavailable)
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Cannot get memory address");
	}
	else // If a local index and script is not running
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "Script '%', is not running", m_scriptName.c_str());
	}
}

void MemWatcherMod::ShowSelectedPopup()
{
	if (ImGui::BeginPopup("PopupEntryProperties"))
	{
		ImGui::Combo("Type##EntryProperties", (int *)&m_selectedEntry->m_type, watchTypeNames, IM_ARRAYSIZE(watchTypeNames));
		ImGui::Checkbox("Show Ingame##EntryProperties", &m_selectedEntry->m_drawInGame);


		if (ImGui::InputText("Info##AddAddress", m_watchInfoModifyBuf, sizeof(m_watchInfoModifyBuf)))
			m_selectedEntry->m_info = std::string(m_watchInfoModifyBuf);
		else if (std::string(m_watchInfoModifyBuf) != m_selectedEntry->m_info)
			strncpy_s(m_watchInfoModifyBuf, sizeof(m_watchInfoModifyBuf), m_selectedEntry->m_info.c_str(), sizeof(m_watchInfoModifyBuf));

		uint64_t * val;
		if (m_selectedEntry->IsGlobal())
		{
			val = getGlobalPtr(m_selectedEntry->m_addressIndex);

			// Can only edit globalPtr for now
			if (val != nullptr)
			{
				switch (m_selectedEntry->m_type)
				{
				case WatchType::kBitfield:
					ImGuiExtras::BitField("Value##GlobalWatchValueBitfield", (unsigned int *)val, nullptr);
					if (ImGui::Button("LS<<##GlobalWatchLBitshift"))
						*val = *val << 1;
					if (ImGui::Button(">>RS##GlobalWatchRBitshift"))
						*val = *val >> 1;
					break;
				case WatchType::kInt:
					ImGui::InputInt("Value##GlobalWatchValue", (int *)val);
					break;
				case WatchType::kFloat:
					ImGui::InputFloat("Value##GlobalWatchValue", (float *)val, 0.0f, 0.0f, "%.4f");
					break;
				case WatchType::kVector3:
					ImGuiExtras::InputVector3("GlobalWatchValue", (Vector3 *)val);
					break;
				case WatchType::kString:
					ImGui::TextDisabled("Cannot edit string.");
					break;
				}
			}
		}
		else
		{
			if (ImGui::InputText("Script Name##EntryProperties", m_scriptNameBuf, sizeof(m_scriptNameBuf)))
			{
				m_scriptName = std::string(m_scriptNameBuf);
				RunOnNativeThread([&]
				{
					m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
					if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
						m_selectedWatchScriptRunning = true;
					else
						m_selectedWatchScriptRunning = false;
				});
			}

			if (m_selectedWatchScriptRunning)
			{
				if (std::string(m_scriptNameBuf) != m_selectedEntry->m_scriptName)
					strncpy_s(m_scriptNameBuf, sizeof(m_scriptNameBuf), m_selectedEntry->m_scriptName.c_str(), sizeof(m_scriptNameBuf));

				m_selectedEntry->m_scriptName = m_scriptName;
				m_selectedEntry->m_scriptHash = m_scriptHash;
				m_selectedWatchScriptRunning = false;
			}
		}

		if (ImGui::Button("Remove##EntryProperties"))
		{
			std::lock_guard<std::mutex> lock(m_watchesMutex);
			m_watches.erase(std::remove(m_watches.begin(), m_watches.end(), m_selectedEntry), m_watches.end());
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void MemWatcherMod::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Watch"))
		{
			if (ImGui::BeginMenu("Add Global Index"))
			{
				ShowAddAddress(true);
				ImGui::EndMenu();
			}
			if (m_supportGlobals)
			{
				if (ImGui::BeginMenu("Add Local Index"))
				{
					ShowAddAddress(false);
					ImGui::EndMenu();
				}
			}

			if (ImGui::MenuItem("Clear"))
			{
				std::lock_guard<std::mutex> lock(m_watchesMutex);
				m_watches.clear();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("HUD"))
		{
			ImGui::MenuItem("Hexadecimal index", NULL, m_inputHexIndex);
			if (ImGui::BeginMenu("Offsets"))
			{
				if (ImGui::InputFloat("X offset", &m_inGameOffsetX, 0.01f))
					ClipFloat(m_inGameOffsetX, 0.0f, 0.95f);
				if (ImGui::InputFloat("Y offset", &m_inGameOffsetY, 0.01f))
					ClipFloat(m_inGameOffsetY, 0.0f, 0.95f);

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

bool MemWatcherMod::Draw()
{
	ImGui::SetWindowFontScale(m_menuFontSize);
	DrawMenuBar();

	ImGui::SetWindowFontScale(m_contentFontSize);
	ImGui::TextColored(ImVec4(255, 0, 0, 255), "Game version: %d", getGameVersion());

	char buf[128] = "";

	ImGui::Columns(5);
	ImGui::Separator();
	ImGui::Text("Index"); ImGui::NextColumn();
	ImGui::Text("Type"); ImGui::NextColumn();
	ImGui::Text("Script"); ImGui::NextColumn();
	ImGui::Text("Info"); ImGui::NextColumn();
	ImGui::Text("Value"); ImGui::NextColumn();
	ImGui::Separator();
	if (m_watches.size() > 0)
	{
		std::lock_guard<std::mutex> lock(m_watchesMutex);
		for (auto &w : m_watches)
		{
			sprintf_s(buf, "%d (0x%x)", w.m_addressIndex, w.m_addressIndex);

			if (ImGui::Selectable(buf, false, ImGuiSelectableFlags_SpanAllColumns))
			{
				m_selectedEntry = &w;
				ImGui::OpenPopup("PopupEntryProperties");
			}

			ImGui::NextColumn();
			ImGui::Text("%s", watchTypeNames[w.m_type]); ImGui::NextColumn();
			ImGui::Text("%s (%d)", w.m_scriptName.c_str(), w.m_scriptHash); ImGui::NextColumn();
			ImGui::Text("%s", w.m_info.c_str()); ImGui::NextColumn();
			ImGui::Text("%s", w.m_value.c_str()); ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
	}

	ShowSelectedPopup();
	return true;
}