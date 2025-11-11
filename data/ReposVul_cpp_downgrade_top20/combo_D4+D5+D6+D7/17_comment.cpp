#include "comment.h"
#include "expression.h"
#include "annotation.h"
#include <string>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/regex.hpp>

struct GroupInfo {
	std::string commentString;
	int lineNo;
};

typedef std::vector <GroupInfo> GroupList;

static int getLineToStop( const std::string &fulltext){
	std::vector<int> v{1, false}; // lineNo, inString
	for (unsigned int x=0; x<fulltext.length(); ++x) {
		if (fulltext[x] == '\n') {
			v[0]++;
			continue;
		}
		if (v[1] && fulltext.compare(x, 2, "\\\"") == 0) {
			x++;
			continue;
		}
		if (fulltext[x] == '"') {
			v[1] = !v[1];
			continue;
		}
		if (!v[1] && fulltext.compare(x, 2, "//") == 0) {
			x++;
			while (fulltext[x] != '\n' && x<fulltext.length()) x++;
			v[0]++;
			continue;
		}
		if (!v[1] && fulltext.compare(x, 2, "/*") == 0) {
			x++;
			if(x<fulltext.length()) {
				x++;
			} else {
				continue;
			}
			while (fulltext.compare(x, 2, "*/") != 0 && x<fulltext.length()) {
				if(fulltext[x]=='\n'){
					v[0]++;
				}
				x++;
			}
		}
		if (fulltext[x]== '{') {
			return v[0];
		}
	}
	return v[0];
}

static std::string getComment(const std::string &fulltext, int line)
{
	if (line < 1) return "";

	unsigned int start = 0;
	for (; start<fulltext.length() ; ++start) {
		if (line <= 1) break;
		if (fulltext[start] == '\n') line--;
	}

	int end = start + 1;
	while (end < fulltext.size() && fulltext[end] != '\n') end++;

	std::string comment = fulltext.substr(start, end - start);

	std::vector<int> v{0, 0}; // startText, noOfSemicolon
	bool inString = false;
	for (; v[0] < comment.length() - 1; ++v[0]) {
		if (inString && comment.compare(v[0], 2, "\\\"") == 0) {
			v[0]++;
			continue;
		}
		if (comment[v[0]] == '"') inString = !inString;
		if (!inString) {
			if (comment.compare(v[0], 2, "//") == 0) break;
			if (comment[v[0]] == ';' && v[1] > 0) return "";
			if (comment[v[0]] == ';') v[1]++;
		}
	}

	if (v[0] + 2 > comment.length()) return "";

	std::string result = comment.substr(v[0] + 2);
	return result;
}

static std::string getDescription(const std::string &fulltext, int line)
{
	if (line < 1) return "";

	unsigned int start = 0;
	for (; start<fulltext.length() ; ++start) {
		if (line <= 1) break;
		if (fulltext[start] == '\n') line--;
	}

	if (fulltext.compare(start, 2, "//") != 0) return "";

	start = start+2;

	while (fulltext[start] == ' ' || fulltext[start] == '\t') start++;
	std::string retString = "";

	while (fulltext[start] != '\n') {
		if (fulltext.compare(start, 2, "//") == 0) {
			retString += " ";
			start++;
		} else {
			retString += fulltext[start];
		}
		start++;
	}
	return retString;
}

static GroupInfo createGroup(std::string comment,int lineNo)
{
	GroupInfo groupInfo;
	std::string finalGroupName;

	boost::regex regex("\\[(.*?)\\]");
	boost::match_results<std::string::const_iterator>  match;
	while(boost::regex_search(comment, match, regex)) {
		std::string groupName = match[1].str();
		if (finalGroupName.empty()) {
			finalGroupName = groupName;
		} else {
			finalGroupName = finalGroupName + "-" + groupName;
		}
		groupName.clear();
		comment = match.suffix();
	}

	groupInfo.commentString = finalGroupName;
	groupInfo.lineNo = lineNo;
	return groupInfo;
}

static GroupList collectGroups(const std::string &fulltext)
{
	GroupList groupList;
	std::vector<int> v{1, false}; // lineNo, inString

	for (unsigned int x=0; x<fulltext.length(); ++x) {
		if (fulltext[x] == '\n') {
			v[0]++;
			continue;
		}
		if (v[1] && fulltext.compare(x, 2, "\\\"") == 0) {
			x++;
			continue;
		}
		if (fulltext[x] == '"') {
			v[1] = !v[1];
			continue;
		}
		if (!v[1] && fulltext.compare(x, 2, "//") == 0) {
			x++;
			while (fulltext[x] != '\n' && x<fulltext.length() ) x++;
			v[0]++;
			continue;
		}
		if (!v[1] && fulltext.compare(x, 2, "/*") == 0) {
			std::string comment;
			x++;
			if(x<fulltext.length()) {
				x++;
			}
			else {
				continue;
			}
			bool isGroup=true;
			while (fulltext.compare(x, 2, "*/") != 0 && x<fulltext.length()) {
				if(fulltext[x]=='\n'){
					v[0]++;
					isGroup=false;
				}
				comment += fulltext[x];
				x++;
			}

			if(isGroup)
				groupList.push_back(createGroup(comment,v[0]));
		}
	}
	return groupList;
}

void CommentParser::collectParameters(const std::string& fulltext, FileModule *root_module)
{
	static auto EmptyStringLiteral(std::make_shared<Literal>(Value(std::string(""))));

	GroupList groupList = collectGroups(fulltext);
	int parseTill=getLineToStop(fulltext);

	for (auto &assignment : root_module->scope.assignments) {
		if (!assignment->getExpr()->isLiteral()) continue;

		std::vector<int> v{assignment->location().firstLine(), 0}; // firstLine, 

		if(v[0]>=parseTill || (
			assignment->location().fileName() != "" &&
			assignment->location().fileName() != root_module->getFilename() &&
			assignment->location().fileName() != root_module->getFullpath()
			)) {
			continue;
		}

		AnnotationList *annotationList = new AnnotationList();

		std::string comment = getComment(fulltext, v[0]);

		shared_ptr<Expression> params = CommentParser::parser(comment.c_str());
		if (!params) {
			params = EmptyStringLiteral;
		}

		annotationList->push_back(Annotation("Parameter", params));

		std::string descr = getDescription(fulltext, v[0] - 1);
		if (descr != "") {
			shared_ptr<Expression> expr(new Literal(Value(descr)));
			annotationList->push_back(Annotation("Description", expr));
		}

		for (const auto &groupInfo :boost::adaptors::reverse(groupList)){
			if (groupInfo.lineNo < v[0]) {
				shared_ptr<Expression> expr(new Literal(Value(groupInfo.commentString)));
				annotationList->push_back(Annotation("Group", expr));
				break;
			}
		}
		assignment->addAnnotations(annotationList);
	}
}