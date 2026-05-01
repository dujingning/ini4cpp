/*
 * MIT License
 *
 * Copyright (c) 2023 dujingning <djn2019x@163.com> <https://github.com/dujingning/inicpp.git>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __JN_INICPP_H__
#define __JN_INICPP_H__

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <type_traits>

#include <fstream>
#include <sstream>

#include <algorithm>
#include <string>
#include <list>
#include <map>

#if defined(_ENABLE_INICPP_STD_WSTRING_) && !defined(_ENBABLE_INICPP_STD_WSTRING_)
#define _ENBABLE_INICPP_STD_WSTRING_
#endif

#ifdef _ENBABLE_INICPP_STD_WSTRING_ // Not all of C++ 11 support <codecvt>
// for std::string <==> std::wstring convert
#include <codecvt>
#include <locale>
#endif

#ifdef INICPP_DEBUG

#include <array>
#include <ctime>
#include <iostream>

class TimeFormatter
{
public:
	static std::string format(const std::string &format = "%Y-%m-%d %H:%M:%S")
	{
		std::time_t t = std::time(nullptr);
		std::tm tm = *std::localtime(&t);
		std::array<char, 100> buffer;
		std::strftime(buffer.data(), buffer.size(), format.c_str(), &tm);
		return buffer.data();
	}
};

#define CODE_INFO std::string(" \t``````|") + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "  fun:" + std::string(__FUNCTION__)
#define INI_DEBUG(x) std::cout << "INICPP " << TimeFormatter::format() << " : " << x << CODE_INFO << std::endl

#else // #ifdef INICPP_DEBUG
#define INI_DEBUG(x)
#endif // #ifdef INICPP_DEBUG

namespace inicpp
{

	typedef struct ValueNode
	{
		std::string Value = "";
		int lineNumber = -1; // text line start with 1
	} ValueNode;

	class parentHelper
	{
	public:
		virtual ~parentHelper() {}

		virtual parentHelper *parent()
		{
			INI_DEBUG("called parentHelper virtual impl: need to impl parent");
			return nullptr;
		}
		virtual void setParent(parentHelper *parent)
		{
			(void)parent;
			INI_DEBUG("called parentHelper virtual impl: need to impl setParent");
		}
		virtual bool set(const std::string &Section, const std::string &Key, const std::string &Value, const std::string &comment = "")
		{
			(void)Section;
			(void)Key;
			(void)Value;
			(void)comment;
			INI_DEBUG("called parentHelper virtual impl: need to impl set");
			return true;
		}
	};

	class ValueProxy
	{
	public:
		ValueProxy(std::string &value) : _value(value) {}
		~ValueProxy() {}

		template <typename T>
		ValueProxy(const T &value) : _value(to_string(value)) {}

		template <typename T>
		static std::string to_string(const T &value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template <typename T>
		T get() const
		{
			static_assert(!std::is_pointer<T>::value, "Pointer types are not supported for conversion.");
			std::istringstream iss(_value);
			T result;
			if (!(iss >> result))
			{
				throw std::runtime_error("Type mismatch or invalid conversion. with(section-key-value): "  +_sectionName +"-"+ _keyName +"-"+ _value); // error notify
			}
			return result;
		}

		operator char() const { return this->get<char>(); }
		operator short() const { return this->get<short>(); }
		operator int() const { return this->get<int>(); }
		operator long() const { return this->get<long>(); }
		operator long long() const { return this->get<long long>(); }
		operator float() const { return this->get<float>(); }
		operator double() const { return this->get<double>(); }

		operator unsigned char() const { return this->get<unsigned char>(); }
		operator unsigned short() const { return this->get<unsigned short>(); }
		operator unsigned int() const { return this->get<unsigned int>(); }
		operator unsigned long() const { return this->get<unsigned long>(); }
		operator unsigned long long() const { return this->get<unsigned long long>(); }

		// false:'0' or 'false', true : others
		operator bool() const
		{
			if (_value == "0" || _value == "false" || _value == "no")
			{
				return false;
			}
			return true;
		}

		operator std::string() const
		{
			return _value;
		}

		friend std::ostream &operator<<(std::ostream &os, const ValueProxy &proxy)
		{
			os << proxy._value;
			return os;
		}

		// assignment
		template <typename T>
		ValueProxy &operator=(const T &other)
		{
			std::string value = this->to_string(other);

			if (_value != value)
			{
				set(value);
			}

			_value = value;
			return *this;
		}

		ValueProxy &operator=(const std::string &other)
		{
			if (_value != other)
			{
				INI_DEBUG("Value Proxy Wanna Set Value: " << other);
				set(other);
			}

			_value = other;
			return *this;
		}

		// specify std::string
		const std::string &String() const noexcept
		{
			return _value;
		}

		inline void setWriteCB(parentHelper *sectionObj, const std::string &sectionName, const std::string &keyName)
		{
			_section = sectionObj;
			_sectionName = sectionName;
			_keyName = keyName;
		}

	private:
		void set(const std::string &value)
		{
			if (value.empty() || _keyName.empty())
			{
				return;
			}
			if (_section && _section->parent() && _section->parent()->parent())
			{
				_section->parent()->parent()->set(_sectionName, _keyName, value);
			}
		}

	private:
		std::string &_value;

		std::string _sectionName, _keyName;
		parentHelper *_section = nullptr;
	};
} // namespace inicpp

namespace inicpp
{

	class section : parentHelper
	{
	public:
		section() : _sectionName()
		{
		}

		explicit section(const std::string &sectionName) : _sectionName(sectionName)
		{
		}

		const std::string &name() const
		{
			return _sectionName;
		}

		const std::string getValue(const std::string &Key) const
		{
			const ValueNode *node = findValue(Key);
			if (!node)
			{
				return "";
			}
			return node->Value;
		}

		void setName(const std::string &name, const int &lineNumber)
		{
			_sectionName = name;
			_lineNumber = lineNumber;
		}

		void setValue(const std::string &Key, const std::string &Value, const int line)
		{
			_sectionMap[Key].Value = Value;
			_sectionMap[Key].lineNumber = line;
		}

		void append(const section &sec)
		{
			_sectionMap.insert(sec._sectionMap.begin(), sec._sectionMap.end());
		}

		bool isKeyExist(const std::string &Key) const
		{
			return findValue(Key) != nullptr;
		}

		int getEndSection() const
		{
			int line = -1;

			if (_sectionMap.empty() && _sectionName != "")
			{
				return _lineNumber;
			}

			for (const auto &data : _sectionMap)
			{
				if (data.second.lineNumber > line)
				{
					line = data.second.lineNumber;
				}
			}
			return line;
		}

		int getLine(const std::string &Key) const
		{
			const ValueNode *node = findValue(Key);
			if (!node)
			{
				return -1;
			}
			return node->lineNumber;
		}

		void clear()
		{
			_lineNumber = -1;
			_sectionName.clear();
			_sectionMap.clear();
		}

		bool isEmpty() const
		{
			return _sectionMap.empty();
		}

		int toInt(const std::string &Key) const noexcept
		{
			const ValueNode *node = findValue(Key);
			if (!node)
			{
				return 0;
			}

			int result = 0;

			try
			{
				result = std::stoi(node->Value);
			}
			catch (const std::invalid_argument &e)
			{
				INI_DEBUG("Invalid argument: " << e.what() << ",input:\'" << node->Value << "\'");
			}
			catch (const std::out_of_range &e)
			{
				INI_DEBUG("Out of range: " << e.what() << ",input:\'" << node->Value << "\'");
			}

			return result;
		}

		std::string toString(const std::string &Key) const noexcept
		{
			const ValueNode *node = findValue(Key);
			if (!node)
			{
				return "";
			}
			return node->Value;
		}

#ifdef _ENBABLE_INICPP_STD_WSTRING_
		std::wstring toWString(const std::string &Key) const
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			return converter.from_bytes(toString(Key));
		}
#endif

		double toDouble(const std::string &Key) const noexcept
		{
			const ValueNode *node = findValue(Key);
			if (!node)
			{
				return 0.0;
			}

			double result = 0.0;

			try
			{
				result = std::stod(node->Value);
			}
			catch (const std::invalid_argument &e)
			{
				INI_DEBUG("Invalid argument: " << e.what() << ",input:\'" << node->Value << "\'");
			}
			catch (const std::out_of_range &e)
			{
				INI_DEBUG("Out of range: " << e.what() << ",input:\'" << node->Value << "\'");
			}

			return result;
		}

		std::map<std::string /*Key*/, std::string /*Value*/> getSectionMap() const
		{
			std::map<std::string /*Key*/, std::string /*Value*/> sectionKVMap;

			for (const auto &iter : _sectionMap)
			{
				sectionKVMap[iter.first] = iter.second.Value;
			}

			return sectionKVMap;
		}

		// Automatically converts to any type; throws std::runtime_error if not found or conversion fails
		ValueProxy operator[](const std::string &Key)
		{
			ValueProxy vp(_sectionMap[Key].Value);

			vp.setWriteCB(this, _sectionName, Key);

			return vp;
		}

		inline parentHelper *parent() override { return _parent; };
		inline void setParent(parentHelper *parent) override { _parent = parent; };

	private:
		ValueNode *findValue(const std::string &Key)
		{
			std::map<std::string, ValueNode>::iterator it = _sectionMap.find(Key);
			if (it == _sectionMap.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		const ValueNode *findValue(const std::string &Key) const
		{
			std::map<std::string, ValueNode>::const_iterator it = _sectionMap.find(Key);
			if (it == _sectionMap.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		std::string _sectionName;
		std::map<std::string /*Key*/, ValueNode> _sectionMap;
		int _lineNumber = -1; // text line start with 1

		parentHelper *_parent = nullptr;
	};

	class ini : parentHelper
	{
	public:
		void addSection(section &sec)
		{
			if (_iniInfoMap.count(sec.name())) // if exist,need to merge
			{
				_iniInfoMap[sec.name()].append(sec);
				return;
			}
			_iniInfoMap.emplace(sec.name(), sec);
			return;
		}

		void removeSection(const std::string &sectionName)
		{
			if (!_iniInfoMap.count(sectionName))
			{
				return;
			}
			_iniInfoMap.erase(sectionName);
			return;
		}

		bool isSectionExists(const std::string &sectionName) const
		{
			return findSection(sectionName) != nullptr;
		}

		// may contains default of Unnamed section with ""
		std::list<std::string> getSectionsList() const
		{
			std::list<std::string> sectionList;
			for (const auto &data : _iniInfoMap)
			{
				if (data.first == "" && data.second.isEmpty()) // default section: no section name,if empty,not count it.
				{
					continue;
				}
				sectionList.emplace_back(data.first);
			}
			return sectionList;
		}

		std::map<std::string /*key*/, std::string /*value*/> getSectionMap(const std::string &sectionName) const
		{
			std::map<std::string /*key*/, std::string /*value*/> kvMap;
			const section *sec = findSection(sectionName);
			if (!sec)
			{
				return kvMap;
			}
			return sec->getSectionMap();
		}

		const section &operator[](const std::string &sectionName)
		{
			_iniInfoMap[sectionName].setParent(this);

			if (_iniInfoMap[sectionName].name().empty())
			{
				_iniInfoMap[sectionName].setName(sectionName, -1);
			}

			return _iniInfoMap[sectionName];
		}

		inline std::size_t getSectionSize() const
		{
			return _iniInfoMap.size();
		}

		std::string getValue(const std::string &sectionName, const std::string &Key) const
		{
			const section *sec = findSection(sectionName);
			if (!sec)
			{
				return "";
			}
			return sec->getValue(Key);
		}

		// for none section
		int getLine(const std::string &Key) const
		{
			const section *sec = findSection("");
			if (!sec)
			{
				return -1;
			}
			return sec->getLine(Key);
		}

		// for section-key
		int getLine(const std::string &sectionName, const std::string &Key) const
		{
			const section *sec = findSection(sectionName);
			if (!sec)
			{
				return -1;
			}
			return sec->getLine(Key);
		}

		inline void clear() { _iniInfoMap.clear(); }
		inline bool empty() const { return _iniInfoMap.empty(); }

		parentHelper *parent() override { return _parent; }
		void setParent(parentHelper *parent) override { _parent = parent; }

	protected:
		std::map<std::string /*Section Name*/, section> _iniInfoMap;

	private:
		section *findSection(const std::string &sectionName)
		{
			std::map<std::string, section>::iterator it = _iniInfoMap.find(sectionName);
			if (it == _iniInfoMap.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		const section *findSection(const std::string &sectionName) const
		{
			std::map<std::string, section>::const_iterator it = _iniInfoMap.find(sectionName);
			if (it == _iniInfoMap.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		parentHelper *_parent = nullptr;
	};

	class IniManager : parentHelper
	{
	public:
#ifdef _ENBABLE_INICPP_STD_WSTRING_
		explicit IniManager(const std::wstring &configFileName = L"")
		{
                        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                        std::string stringValue = converter.to_bytes(configFileName);
			_configFileName = stringValue;

			_iniData.setParent(this);
			parse();
		}
#else
		explicit IniManager(const std::string &configFileName = "") : _configFileName(configFileName)
		{
			_iniData.setParent(this);

			parse();
		}
#endif
		~IniManager()
		{
			_iniFile.close();
		}

		section operator[](const std::string &sectionName)
		{
			return _iniData[sectionName];
		}

		void parse()
		{
			if (_configFileName.empty())
			{
				return;
			}

			std::fstream input;
			input.open(_configFileName.c_str(), std::ifstream::in | std::ifstream::out | std::fstream::app);

			if (!input.is_open())
			{
				INI_DEBUG("Failed to open(WR),try to open with readonly(R).");
				input.clear();
				input.open(_configFileName.c_str(), std::ifstream::in);
			}

			if (!input.is_open())
			{
				INI_DEBUG("Failed to open the input INI file for parsing! file:" << _configFileName);
				return;
			}

			ini parsed;
			parsed.setParent(this);

			input.seekg(0, input.beg);
			std::string data, sectionName;
			int sectionLine = -1;

			section sectionRecord;

			_SumOfLines = 1;
			while (std::getline(input, data))
			{
				if (!filterData(data))
				{
					++_SumOfLines;
					continue;
				}

				if (data.find('[') == 0) // section
				{
					if (!sectionRecord.isEmpty() || sectionRecord.name() != "")
					{
						parsed.addSection(sectionRecord);
					}

					size_t first = data.find('[');
					size_t last = data.find(']');

					if (last == std::string::npos)
					{
						++_SumOfLines;
						continue;
					}

					sectionName = data.substr(first + 1, last - first - 1);
					sectionLine = _SumOfLines;

					sectionRecord.clear();
					sectionRecord.setName(sectionName, sectionLine);
				}

				size_t pos = data.find('=');
				if (pos != std::string::npos)
				{ // k=v
					std::string key = data.substr(0, pos);
					std::string value = data.substr(pos + 1);

					trimEdges(key);
					trimEdges(value);

					sectionRecord.setValue(key, value, _SumOfLines);
				}

				++_SumOfLines;
			}

			if (!sectionRecord.isEmpty())
			{
				sectionRecord.setName(sectionName, -1);
				parsed.addSection(sectionRecord);
			}

			_iniData = parsed;
		}

		bool set(const std::string &Section, const std::string &Key, const std::string &Value, const std::string &comment = "") override
		{
			parse();

			std::string key = Key, value = Value;

			trimEdges(key);
			trimEdges(key);

			if (key == "" || value == "")
			{
				INI_DEBUG("Invalid parameter input: key[" << key << "],value[" << value << "]");
				return false;
			}

			std::string keyValueData = key + "=" + value + "\n";
			if (comment.length() > 0)
			{
				keyValueData = comment + "\n" + keyValueData;
				if (comment[0] != ';')
				{
					keyValueData = ";" + keyValueData;
				}
			}

			const std::string &tempFile = ".temp.ini";
			std::fstream input(_configFileName, std::ifstream::in);
			std::ofstream output(tempFile);

			if (!input.is_open())
			{
				INI_DEBUG("Failed to open the input INI file for modification! File name:" << _configFileName);
				return false;
			}

			if (!output.is_open())
			{
				INI_DEBUG("Failed to open the output INI file for modification!");
				return false;
			}

			int line_number_mark = -1;
			bool isInputDataWited = false;

			do
			{
				// exist key at one section replace it, or need to create it
				if (_iniData.isSectionExists(Section))
				{
					line_number_mark = (*this)[Section].getLine(key);

					if (line_number_mark == -1)
					{ // section exist, key not exist
						line_number_mark = (*this)[Section].getEndSection();

						std::string lineData;
						int input_line_number = 0;
						while (std::getline(input, lineData))
						{
							++input_line_number;

							if (input_line_number == (line_number_mark + 1))
							{ // new line,append to next line
								isInputDataWited = true;
								output << keyValueData;
							}

							output << lineData << "\n";
						}

						if (input.eof() && !isInputDataWited)
						{
							isInputDataWited = true;
							output << keyValueData;
						}

						break;
					}
				}

				if (line_number_mark <= 0) // not found key at config file
				{
					input.seekg(0, input.beg);

					bool isHoldSection = false;
					std::string newLine = "\n\n";
					if (Section != "" && Section.find("[") == std::string::npos && Section.find("]") == std::string::npos && Section.find("=") == std::string::npos)
					{
						if (_iniData.empty() || _iniData.getSectionSize() <= 0)
						{
							newLine.clear();
						}

						isHoldSection = true;
					}

					// 1.section is exist or empty section
					if (_iniData.isSectionExists(Section) || Section == "")
					{
						// write key/value to head
						if (isHoldSection)
						{
							output << newLine << "[" << Section << "]" << "\n";
						}
						output << keyValueData;
						// write others
						std::string lineData;
						while (std::getline(input, lineData))
						{
							output << lineData << "\n";
						}
					}
					// 2.section is not exist
					else
					{
						// write others
						std::string lineData;
						while (std::getline(input, lineData))
						{
							output << lineData << "\n";
						}
						// write key/value to end
						if (isHoldSection)
						{
							output << newLine << "[" << Section << "]" << "\n";
						}
						output << keyValueData;
					}

					break;
				}
				else
				{ // found, replace it

					std::string lineData;
					int input_line_number = 0;

					while (std::getline(input, lineData))
					{
						++input_line_number;

						// delete old comment if new comment is set
						if (input_line_number == (line_number_mark - 1) && lineData.length() > 0 && lineData[0] == ';' && comment != "")
						{
							continue;
						}

						if (input_line_number == line_number_mark)
						{ // replace to this line
							output << keyValueData;
						}
						else
						{
							output << lineData << "\n";
						}
					}
					break;
				}

				INI_DEBUG("error! inicpp lost process of set function");
				return false;

			} while (false);

			// clear work
			input.close();
			output.close();

			std::remove(_configFileName.c_str());
			std::rename(tempFile.c_str(), _configFileName.c_str());

			// reload
			parse();

			return true;
		}

		bool set(const std::string &Section, const std::string &Key, const int Value, const std::string &comment = "")
		{
			std::string stringValue = std::to_string(Value);
			return set(Section, Key, stringValue, comment);
		}

		bool set(const std::string &Section, const std::string &Key, const double &Value, const std::string &comment = "")
		{
			std::string stringValue = std::to_string(Value);
			return set(Section, Key, stringValue, comment);
		}

		bool set(const std::string &Section, const std::string &Key, const char &Value, const std::string &comment = "")
		{
			std::string stringValue = ValueProxy::to_string(Value);
			return set(Section, Key, stringValue, comment);
		}

		// no sections: head of config file
		bool set(const std::string &Key, const std::string &Value)
		{
			return set("", Key, Value, "");
		}
		bool set(const std::string &Key, const char *Value)
		{
			return set("", Key, Value, "");
		}
		template <typename T>
		bool set(const std::string &Key, const T &Value)
		{
			std::string stringValue = std::to_string(Value);
			return set("", Key, stringValue, "");
		}

#ifdef _ENBABLE_INICPP_STD_WSTRING_
		bool set(const std::string &Section, const std::string &Key, const std::wstring &Value, const std::string &comment = "")
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::string stringValue = converter.to_bytes(Value);

			return set(Section, Key, stringValue, comment);
		}
#endif
		// comment for section name of key
		bool setComment(const std::string &Section, const std::string &Key, const std::string &comment)
		{
			return set(Section, Key, (*this)[Section].toString(Key), comment);
		}
		// comment for no section name of key
		bool setComment(const std::string &Key, const std::string &comment)
		{
			return set("", Key, (*this)[""].toString(Key), comment);
		}

		bool isSectionExists(const std::string &sectionName) const
		{
			return _iniData.isSectionExists(sectionName);
		}

		inline std::list<std::string /*section name*/> sectionsList() const
		{
			return _iniData.getSectionsList();
		}

		inline std::map<std::string /*key*/, std::string /*value*/> sectionMap(const std::string &sectionName) const
		{
			return _iniData.getSectionMap(sectionName);
		}

#ifdef _ENBABLE_INICPP_STD_WSTRING_
		void setFileName(const std::wstring &fileName)
		{
                        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                        std::string name = converter.to_bytes(fileName);

			_configFileName = name;
		}
#else
		void setFileName(const std::string &fileName)
		{
			_configFileName = fileName;
		}
#endif

	private:
		bool filterData(std::string &data)
		{
			if (data.length() == 0)
			{
				return false;
			}

			if (data[0] == ';')
			{
				return false;
			}

			if (data[0] == '#')
			{
				return false;
			}

			return true;
		}

		void trimEdges(std::string &data)
		{
			// remove left ' ' and '\t'
			data.erase(data.begin(), std::find_if(data.begin(), data.end(), [](unsigned char c)
												  { return !std::isspace(c); }));
			// remove right ' ' and '\t'
			data.erase(std::find_if(data.rbegin(), data.rend(), [](unsigned char c)
									{ return !std::isspace(c); })
						   .base(),
					   data.end());

			// INI_DEBUG("trimEdges data:|" << data << "|");
		}

	private:
		ini _iniData;
		int _SumOfLines;
		std::fstream _iniFile;
		std::string _configFileName;
	};

} // namespace inicpp

#endif
