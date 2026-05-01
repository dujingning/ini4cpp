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

#ifndef INICPP_HPP
#define INICPP_HPP

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

namespace inicpp
{
namespace debug
{

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

} // namespace debug
} // namespace inicpp

#define INICPP_CODE_INFO std::string(" \t``````|") + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "  fun:" + std::string(__func__)
#define INICPP_DEBUG_LOG(x) std::cout << "INICPP " << ::inicpp::debug::TimeFormatter::format() << " : " << x << INICPP_CODE_INFO << std::endl

#else // #ifdef INICPP_DEBUG
#define INICPP_DEBUG_LOG(x)
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
			INICPP_DEBUG_LOG("called parentHelper virtual impl: need to impl parent");
			return nullptr;
		}
		virtual void setParent(parentHelper *parent)
		{
			(void)parent;
			INICPP_DEBUG_LOG("called parentHelper virtual impl: need to impl setParent");
		}
		virtual bool set(const std::string &Section, const std::string &Key, const std::string &Value, const std::string &comment = "")
		{
			(void)Section;
			(void)Key;
			(void)Value;
			(void)comment;
			INICPP_DEBUG_LOG("called parentHelper virtual impl: need to impl set");
			return true;
		}
	};

	class ValueProxy
	{
	public:
		ValueProxy(std::string &value) : _value(value) {}
		~ValueProxy() {}

		template <typename T>
		ValueProxy(const T &value) = delete;

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
			return getValue<T>();
		}

	private:
		template <typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type getValue() const
		{
			return _value;
		}

		template <typename T>
		typename std::enable_if<!std::is_same<T, std::string>::value, T>::type getValue() const
		{
			std::istringstream iss(_value);
			T result;
			if (!(iss >> result))
			{
				throw std::runtime_error("Type mismatch or invalid conversion. with(section-key-value): "  +_sectionName +"-"+ _keyName +"-"+ _value); // error notify
			}
			return result;
		}

	public:
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
				INICPP_DEBUG_LOG("Value Proxy Wanna Set Value: " << other);
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
			if (_keyName.empty())
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
			for (std::map<std::string, ValueNode>::const_iterator it = sec._sectionMap.begin(); it != sec._sectionMap.end(); ++it)
			{
				_sectionMap[it->first] = it->second;
			}
		}

		bool isKeyExists(const std::string &Key) const
		{
			return findValue(Key) != nullptr;
		}

		bool isKeyExist(const std::string &Key) const
		{
			return isKeyExists(Key);
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
			return toIntOrDefault(node->Value);
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
			return toDoubleOrDefault(node->Value);
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

		static int toIntOrDefault(const std::string &value) noexcept
		{
			int result = 0;
			try
			{
				result = std::stoi(value);
			}
			catch (const std::invalid_argument &e)
			{
				INICPP_DEBUG_LOG("Invalid argument: " << e.what() << ",input:'" << value << "'");
			}
			catch (const std::out_of_range &e)
			{
				INICPP_DEBUG_LOG("Out of range: " << e.what() << ",input:'" << value << "'");
			}
			return result;
		}

		static double toDoubleOrDefault(const std::string &value) noexcept
		{
			double result = 0.0;
			try
			{
				result = std::stod(value);
			}
			catch (const std::invalid_argument &e)
			{
				INICPP_DEBUG_LOG("Invalid argument: " << e.what() << ",input:'" << value << "'");
			}
			catch (const std::out_of_range &e)
			{
				INICPP_DEBUG_LOG("Out of range: " << e.what() << ",input:'" << value << "'");
			}
			return result;
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

			std::ifstream input(_configFileName.c_str(), std::ifstream::in);
			if (!input.is_open())
			{
				INICPP_DEBUG_LOG("Failed to open the input INI file for parsing! file:" << _configFileName);
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
				stripUtf8Bom(data, _SumOfLines == 1);

				if (!filterData(data))
				{
					++_SumOfLines;
					continue;
				}

				if (parseSectionHeader(data, sectionName)) // section
				{
					if (!sectionRecord.isEmpty() || sectionRecord.name() != "")
					{
						parsed.addSection(sectionRecord);
					}

					sectionLine = _SumOfLines;

					sectionRecord.clear();
					sectionRecord.setName(sectionName, sectionLine);
					++_SumOfLines;
					continue;
				}

				std::string key, value;
				if (parseKeyValueLine(data, key, value))
				{ // k=v
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
			return setValue(Section, Key, Value, comment, false);
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
			return setValue(Section, Key, (*this)[Section].toString(Key), comment, true);
		}
		// comment for no section name of key
		bool setComment(const std::string &Key, const std::string &comment)
		{
			return setValue("", Key, (*this)[""].toString(Key), comment, true);
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
		bool setValue(const std::string &Section, const std::string &Key, const std::string &Value, const std::string &comment, const bool allowEmptyValue)
		{
			std::string key = Key, value = Value;

			trimEdges(key);

			if (key == "")
			{
				INICPP_DEBUG_LOG("Invalid parameter input: key[" << key << "],value[" << value << "]");
				return false;
			}

			(void)allowEmptyValue;

			if (!ensureFileExists(_configFileName))
			{
				return false;
			}

			parse();

			std::string updatedContent;
			if (!buildUpdatedFileContent(Section, key, value, comment, updatedContent))
			{
				return false;
			}

			if (!replaceFileWithBackup(_configFileName, updatedContent))
			{
				return false;
			}

			// reload
			parse();

			return true;
		}

		bool buildUpdatedFileContent(const std::string &Section, const std::string &key, const std::string &value, const std::string &comment, std::string &content)
		{
			std::ifstream file(_configFileName.c_str(), std::ifstream::in | std::ifstream::binary);

			if (!file.is_open())
			{
				INICPP_DEBUG_LOG("Failed to open the input INI file for modification! File name:" << _configFileName);
				return false;
			}

			std::ostringstream fileBuffer;
			fileBuffer << file.rdbuf();
			if (file.bad() || fileBuffer.bad())
			{
				return false;
			}

			const std::string originalContent = fileBuffer.str();
			const std::string lineEnding = detectLineEnding(originalContent);
			const std::string keyValueData = formatKeyValueData(key, value, comment, lineEnding);

			std::istringstream input(originalContent);
			std::ostringstream output;

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
							stripTrailingCarriageReturn(lineData);
							++input_line_number;

							if (input_line_number == (line_number_mark + 1))
							{ // new line,append to next line
								isInputDataWited = true;
								output << keyValueData;
							}

							output << lineData << lineEnding;
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
					std::string newLine = lineEnding + lineEnding;
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
							output << newLine << "[" << Section << "]" << lineEnding;
						}
						output << keyValueData;
						// write others
						std::string lineData;
						while (std::getline(input, lineData))
						{
							stripTrailingCarriageReturn(lineData);
							output << lineData << lineEnding;
						}
					}
					// 2.section is not exist
					else
					{
						// write others
						std::string lineData;
						while (std::getline(input, lineData))
						{
							stripTrailingCarriageReturn(lineData);
							output << lineData << lineEnding;
						}
						// write key/value to end
						if (isHoldSection)
						{
							output << newLine << "[" << Section << "]" << lineEnding;
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
						stripTrailingCarriageReturn(lineData);
						++input_line_number;

						// delete old comment if new comment is set
						if (input_line_number == (line_number_mark - 1) && isCommentLine(lineData) && comment != "")
						{
							continue;
						}

						if (input_line_number == line_number_mark)
						{ // replace to this line
							output << keyValueData;
						}
						else
						{
							output << lineData << lineEnding;
						}
					}
					break;
				}

				INICPP_DEBUG_LOG("error! inicpp lost process of set function");
				return false;

			} while (false);

			if (input.bad() || !output)
			{
				return false;
			}

			content = output.str();
			return true;
		}

		static bool ensureFileExists(const std::string &fileName)
		{
			if (fileName.empty())
			{
				return false;
			}

			std::ifstream input(fileName.c_str(), std::ifstream::in | std::ifstream::binary);
			if (input.good())
			{
				return true;
			}

			std::ofstream output(fileName.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app);
			return output.good();
		}

		static std::string detectLineEnding(const std::string &content)
		{
			const std::string::size_type lf = content.find('\n');
			if (lf != std::string::npos && lf > 0 && content[lf - 1] == '\r')
			{
				return "\r\n";
			}
			return "\n";
		}

		static void stripTrailingCarriageReturn(std::string &line)
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
			{
				line.erase(line.size() - 1);
			}
		}

		static bool hasCommentMarker(const std::string &comment)
		{
			const std::string::size_type pos = firstNonSpace(comment);
			return pos < comment.size() && (comment[pos] == ';' || comment[pos] == '#');
		}

		static std::string formatKeyValueData(const std::string &key, const std::string &value, const std::string &comment, const std::string &lineEnding)
		{
			std::ostringstream data;
			if (!comment.empty())
			{
				if (!hasCommentMarker(comment))
				{
					data << ";";
				}
				data << comment << lineEnding;
			}
			data << key << "=" << value << lineEnding;
			return data.str();
		}

		static bool replaceFileWithBackup(const std::string &fileName, const std::string &content)
		{
			if (fileName.empty())
			{
				return false;
			}

			const std::string tempFile = fileName + ".inicpp.tmp";
			const std::string backupFile = fileName + ".inicpp.bak";

			{
				std::ofstream output(tempFile.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
				if (!output.is_open())
				{
					return false;
				}

				output << content;
				output.close();
				if (!output)
				{
					std::remove(tempFile.c_str());
					return false;
				}
			}

			{
				std::ifstream backupInput(backupFile.c_str());
				if (backupInput.good())
				{
					backupInput.close();
					if (std::remove(backupFile.c_str()) != 0)
					{
						std::remove(tempFile.c_str());
						return false;
					}
				}
			}

			if (std::rename(fileName.c_str(), backupFile.c_str()) != 0)
			{
				std::remove(tempFile.c_str());
				return false;
			}

			if (std::rename(tempFile.c_str(), fileName.c_str()) != 0)
			{
				if (std::rename(backupFile.c_str(), fileName.c_str()) != 0)
				{
					INICPP_DEBUG_LOG("Failed to restore original INI file from backup! File name:" << fileName);
				}
				std::remove(tempFile.c_str());
				return false;
			}

			if (std::remove(backupFile.c_str()) != 0)
			{
				INICPP_DEBUG_LOG("Failed to remove backup file: " << backupFile);
			}

			return true;
		}

		bool filterData(const std::string &data) const
		{
			if (data.length() == 0)
			{
				return false;
			}

			if (isCommentLine(data))
			{
				return false;
			}

			return true;
		}

		static bool isSpace(const char c)
		{
			return std::isspace(static_cast<unsigned char>(c)) != 0;
		}

		static std::string::size_type firstNonSpace(const std::string &data)
		{
			std::string::size_type pos = 0;
			while (pos < data.size() && isSpace(data[pos]))
			{
				++pos;
			}
			return pos;
		}

		bool isCommentLine(const std::string &data) const
		{
			const std::string::size_type pos = firstNonSpace(data);
			return pos < data.size() && (data[pos] == ';' || data[pos] == '#');
		}

		static void stripUtf8Bom(std::string &data, const bool isFirstLine)
		{
			if (!isFirstLine || data.size() < 3)
			{
				return;
			}

			if (static_cast<unsigned char>(data[0]) == 0xEF &&
				static_cast<unsigned char>(data[1]) == 0xBB &&
				static_cast<unsigned char>(data[2]) == 0xBF)
			{
				data.erase(0, 3);
			}
		}

		static std::string::size_type findInlineCommentStart(const std::string &data)
		{
			bool inSingleQuote = false;
			bool inDoubleQuote = false;
			bool escaped = false;

			for (std::string::size_type i = 0; i < data.size(); ++i)
			{
				const char c = data[i];

				if (escaped)
				{
					escaped = false;
					continue;
				}

				if (c == '\\')
				{
					escaped = true;
					continue;
				}

				if (c == '\'' && !inDoubleQuote)
				{
					inSingleQuote = !inSingleQuote;
					continue;
				}

				if (c == '"' && !inSingleQuote)
				{
					inDoubleQuote = !inDoubleQuote;
					continue;
				}

				if ((c == ';' || c == '#') && !inSingleQuote && !inDoubleQuote)
				{
					if (i == 0 || isSpace(data[i - 1]))
					{
						return i;
					}
				}
			}

			return std::string::npos;
		}

		void stripInlineComment(std::string &data)
		{
			const std::string::size_type commentStart = findInlineCommentStart(data);
			if (commentStart != std::string::npos)
			{
				data.erase(commentStart);
				trimEdges(data);
			}
		}

		bool parseSectionHeader(const std::string &line, std::string &sectionName)
		{
			std::string data = line;
			stripInlineComment(data);
			trimEdges(data);

			if (data.empty() || data[0] != '[')
			{
				return false;
			}

			const std::string::size_type last = data.find(']');
			if (last == std::string::npos)
			{
				return false;
			}

			std::string tail = data.substr(last + 1);
			trimEdges(tail);
			if (!tail.empty())
			{
				return false;
			}

			std::string parsedName = data.substr(1, last - 1);
			trimEdges(parsedName);
			if (parsedName.empty())
			{
				return false;
			}

			sectionName = parsedName;
			return true;
		}

		static std::string::size_type findKeyValueDelimiter(const std::string &data)
		{
			const std::string::size_type equals = data.find('=');
			if (equals != std::string::npos)
			{
				return equals;
			}
			return data.find(':');
		}

		bool parseKeyValueLine(const std::string &line, std::string &key, std::string &value)
		{
			std::string data = line;
			stripInlineComment(data);

			const std::string::size_type pos = findKeyValueDelimiter(data);
			if (pos == std::string::npos)
			{
				return false;
			}

			key = data.substr(0, pos);
			value = data.substr(pos + 1);

			trimEdges(key);
			trimEdges(value);

			return !key.empty();
		}

		static void trimEdges(std::string &data)
		{
			// remove left ' ' and '\t'
			data.erase(data.begin(), std::find_if(data.begin(), data.end(), [](unsigned char c)
												  { return !std::isspace(c); }));
			// remove right ' ' and '\t'
			data.erase(std::find_if(data.rbegin(), data.rend(), [](unsigned char c)
									{ return !std::isspace(c); })
						   .base(),
					   data.end());

			// INICPP_DEBUG_LOG("trimEdges data:|" << data << "|");
		}

	private:
		ini _iniData;
		int _SumOfLines;
		std::fstream _iniFile;
		std::string _configFileName;
	};

} // namespace inicpp

#endif // INICPP_HPP
