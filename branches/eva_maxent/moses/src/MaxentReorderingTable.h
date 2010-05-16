#ifndef MAXENTREORDERINGTABLE_H_
#define MAXENTREORDERINGTABLE_H_

//stdlib dependencies:
#include <vector>
#include <map>
#include <string>
#include <iostream>
//moses dependencies:
#include "TypeDef.h"
#include "Phrase.h"
#include "InputType.h"
#include "ConfusionNet.h"
#include "Sentence.h"
#include "PrefixTreeMap.h"

namespace Moses
{

class Phrase;
class InputType;
class ConfusionNet;

//additional types

typedef std::vector<float>      Score;
typedef std::vector<FactorType> FactorList;

class MaxentReorderingTable {
 public:
 MaxentReorderingTable(const FactorList& f_factors, const FactorList& e_factors, const FactorList& c_factors)
   : m_FactorsF(f_factors), m_FactorsE(e_factors), m_FactorsC(c_factors) {
  }
  virtual ~MaxentReorderingTable(){
  }
 public:
  static MaxentReorderingTable* LoadAvailable(const std::string& filePath, const FactorList& f_factors, const FactorList& e_factors, const FactorList& c_factors);
 public:
  virtual Score GetScore(const Phrase& f, const Phrase& e, const Phrase& c, const Phrase& f_context) = 0;
  virtual void InitializeForInput(const InputType&){
    /* override for on-demand loading */
  };
  virtual void InitializeForInputPhrase(const Phrase&){
  };
  /*
  int GetNumScoreComponents() const {
    return m_NumScores;
  }
  */
  const FactorList& GetFFactorMask() const {
    return m_FactorsF;
  }
  const FactorList& GetEFactorMask() const {
    return m_FactorsE;
  }
  const FactorList& GetCFactorMask() const {
	return m_FactorsC;
  }
  virtual void DbgDump(std::ostream* out) const{
	*out << "Overwrite in subclass...\n";
  };
 protected:
  FactorList m_FactorsF;
  FactorList m_FactorsE;
  FactorList m_FactorsC;
};

class MaxentReorderingTableMemory : public MaxentReorderingTable {
  //implements LexicalReorderingTable saving all scores in one large std::map<> thingy
  //to be used for non binary tables... uses a LOT of memory
 public:
  MaxentReorderingTableMemory( const std::string& filePath,
				const std::vector<FactorType>& f_factors, 
                                const std::vector<FactorType>& e_factors,
				const std::vector<FactorType>& c_factors);
  virtual ~MaxentReorderingTableMemory();
 public:
  virtual std::vector<float> GetScore(const Phrase& f, const Phrase& e, const Phrase& c, const Phrase& f_context);
  void DbgDump(std::ostream* out) const;
 private:
  std::string MakeKey(const Phrase& f, const Phrase& e, const Phrase& c, const Phrase& f_context) const;
  std::string MakeKey(const std::string& f, const std::string& e, const std::string& c, const std::string& f_context) const;
  
  void LoadFromFile(const std::string& filePath);
 private:
  typedef std::map< std::string, std::vector<float> > TableType;
  TableType m_Table; 
};

class MaxentReorderingTableTree : public MaxentReorderingTable {
  //implements LexicalReorderingTable using the crafty PDT code...
 public:
  MaxentReorderingTableTree(const std::string& filePath,
							 const std::vector<FactorType>& f_factors, 
							 const std::vector<FactorType>& e_factors,
							 const std::vector<FactorType>& c_factors);
  ~MaxentReorderingTableTree();
 public:
  bool IsCacheEnabled() const {
    return m_UseCache;
  };
  void EnableCache() {
    m_UseCache = true;
  };
  void DisableCache() {
    m_UseCache = false;
  };
  void ClearCache(){
	m_Cache.clear();
  };

  virtual std::vector<float> GetScore(const Phrase& f, const Phrase& e, const Phrase& c, const Phrase& f_context);

  virtual void InitializeForInput(const InputType& input);
  virtual void InitializeForInputPhrase(const Phrase& f){
	ClearCache();
	auxCacheForSrcPhrase(f);
  }
 public:
  static bool Create(std::istream& inFile, const std::string& outFileName);
 private:
  std::string MakeCacheKey(const Phrase& f, const Phrase& e) const;
  IPhrase     MakeTableKey(const Phrase& f, const Phrase& e) const;
  
  void Cache(const ConfusionNet& input);
  void Cache(const Sentence& input);

  void  auxCacheForSrcPhrase(const Phrase& f);
  Score auxFindScoreForContext(const Candidates& cands, const Phrase& contex);
 private:
  //typedef LexicalReorderingCand          CandType;
  typedef std::map< std::string, Candidates > CacheType;
  typedef PrefixTreeMap        TableType;
  
  static const int SourceVocId = 0;
  static const int TargetVocId = 1;

  bool      m_UseCache;
  CacheType m_Cache;
  TableType m_Table;
};

}

#endif /*MAXENTREORDERINGTABLE_H_*/