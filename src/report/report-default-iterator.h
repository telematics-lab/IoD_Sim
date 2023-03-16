#ifndef REPORT_DEFAULT_ITERATOR_H
#define REPORT_DEFAULT_ITERATOR_H

#include <string>

#include <ns3/type-id.h>

namespace ns3 {

class ReportDefaultIterator
{
public:
  virtual ~ReportDefaultIterator () = 0;
  /**
   * \brief This function will go through all the TypeIds and get only the attributes which are
   * explicit values (not vectors or pointer or arrays) and apply StartVisitTypeId
   * and VisitAttribute on the attributes in one TypeId. At the end of each TypeId
   * EndVisitTypeId is called.
   */
  void Iterate ();
private:
  /**
   * \brief Just an interface that needs to be implemented
   */
  virtual void StartVisitTypeId (std::string name);
  /**
   * \brief Just an interface that needs to be implemented
   */
  virtual void EndVisitTypeId ();
  /**
   * \brief This method can be implemented, otherwise, it will call DoVisitAttribute
   */
  virtual void VisitAttribute (TypeId tid, std::string name, std::string defaultValue, uint32_t index);
  /**
   * \brief This method is just an interface and needs to be implemented
   */
  virtual void DoVisitAttribute (std::string name, std::string defaultValue);
};

} // namespace ns3

#endif /* REPORT_DEFAULT_ITERATOR_H */