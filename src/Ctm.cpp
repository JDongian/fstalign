/*
*
* Ctm.cpp
*
* JP Robichaud (jp@rev.com)
  2018
*
*/

#include "Ctm.h"

#include <csv/csv.h>

using namespace std;
using namespace fst;

/***************************************
    CTM FST Loader Class Start
 ***************************************/
CtmFstLoader::CtmFstLoader(vector<RawCtmRecord> &records, bool keep_case) : FstLoader() {
  {
    keep_case_ = keep_case;
    mCtmRows = records;
    for (auto &row : mCtmRows) {
      string token = string(row.word);
      if (!keep_case_) {
          std::transform(token.begin(), token.end(), token.begin(), ::tolower);
      }
      mToken.push_back(token);
    }
  }
}

CtmFstLoader::~CtmFstLoader() {
  // TODO Auto-generated destructor stub
}
void CtmFstLoader::addToSymbolTable(SymbolTable &symbol) const {
  for (auto &s : mToken) {
    AddSymbolIfNeeded(symbol, s);
  }
}

StdVectorFst CtmFstLoader::convertToFst(const SymbolTable &symbol) const {
  auto logger = logger::GetOrCreateLogger("ctmloader");
  //
  StdVectorFst transducer;
  logger->debug("creating transducer for CTM");

  transducer.AddState();
  transducer.SetStart(0);

  int prevState = 0;
  int nextState = 1;
  for (TokenType::const_iterator i = mToken.begin(); i != mToken.end(); ++i) {
    std::string token = *i;
    if (!keep_case_) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
    }
    transducer.AddState();

    transducer.AddArc(prevState, StdArc(symbol.Find(token), symbol.Find(token), 0.0f, nextState));
    prevState = nextState;
    nextState++;
  }

  transducer.SetFinal(prevState, 0.0f);
  return transducer;
}

/***************************************
      CTM FST Loader Class End
   ***************************************/

/***************************************
      CTM Reader Class Start
   ***************************************/
CtmReader::CtmReader() {}

vector<RawCtmRecord> read_from_disk_no_conf(const string &filename) {
  vector<RawCtmRecord> vect;
  io::CSVReader<5, io::trim_chars<' ', '\t'>, io::no_quote_escape<' '>, io::throw_on_overflow, io::empty_line_comment>
      input_ctm(filename);

  input_ctm.set_header("audiofile", "channel", "start", "duration", "word");

  string audiofile, channel, start, duration, word, confidence;
  while (input_ctm.read_row(audiofile, channel, start, duration, word)) {
    RawCtmRecord record;
    record.recording = audiofile;
    record.channel = channel;
    record.start_time_secs = stof(start);
    record.duration_secs = stof(duration);
    record.word = word;
    record.confidence = 1;
    vect.push_back(record);
  }

  return vect;
}

vector<RawCtmRecord> read_from_disk_with_conf(const string &filename) {
  vector<RawCtmRecord> vect;
  io::CSVReader<6, io::trim_chars<' ', '\t'>, io::no_quote_escape<' '>, io::throw_on_overflow, io::empty_line_comment>
      input_ctm(filename);

  input_ctm.set_header("audiofile", "channel", "start", "duration", "word", "confidence");

  string audiofile, channel, start, duration, word, confidence;
  while (input_ctm.read_row(audiofile, channel, start, duration, word, confidence)) {
    RawCtmRecord record;
    record.recording = audiofile;
    record.channel = channel;
    record.start_time_secs = stof(start);
    record.duration_secs = stof(duration);
    record.word = word;
    record.confidence = stof(confidence);
    vect.push_back(record);
  }

  return vect;
}

vector<RawCtmRecord> CtmReader::read_from_disk(const string &filename) {
  ifstream ctm_peek(filename);
  string first_line;
  if (!std::getline(ctm_peek, first_line)) {
    vector<RawCtmRecord> vect;
    return vect;
  }

  int sz = 1;
  char lastChar = 'x';

  for (auto &c : first_line) {
    if (c == ' ' || c == '\t') {
      if (lastChar != ' ' && lastChar != '\t') {
        sz++;
      }
    }

    lastChar = c;
  }

  // Minimum CTM columns should be: audiofile, channel, start, duration, word
  // Sixth confidence score column is optional
  bool hasConf = sz > 5 ? true : false;

  if (hasConf) {
    return read_from_disk_with_conf(filename);
  } else {
    return read_from_disk_no_conf(filename);
  }
}

/***************************************
      CTM Reader Class End
   ***************************************/
