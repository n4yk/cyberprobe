
#ifndef BASE_CONTEXT_H
#define BASE_CONTEXT_H

#include <exception>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <map>

#include "flow.h"
#include "thread.h"
#include "exception.h"

namespace analyser {

    // Context ID type.
    typedef unsigned long context_id;

    // Forward declarations.
    class base_context;

    // Shared pointer types.
    typedef boost::shared_ptr<base_context> context_ptr;

    // Context class, describes the state around a 'flow' of data between
    // two endpoints at a particular network layer.
    class base_context {
      private:

	// Next context ID to hand out.
	static context_id next_context_id;
	static unsigned long total_contexts;

	// This context's ID.
	context_id id;

      public:

	// The flow address.
	flow addr;

	// Lock for all context state.
	threads::mutex lock;

	// Use weak_ptr for the parent link, cause otherwise there's a
	// shared_ptr cycle.
	boost::weak_ptr<base_context> parent;

	// Child contexts.
	std::map<flow,context_ptr> children;

	// Constructor.
        base_context() { 
	    id = next_context_id++; 
	    total_contexts++;
	    // parent is initialised to 'null'.
	}

	// Constructor, initialises parent pointer.
        base_context(context_ptr parent) {
	    id = next_context_id++; 
	    this->parent = parent;
	    total_contexts++;
	}

	// Given a flow address, returns the child context.
	context_ptr get_child(const flow& f) {
	    lock.lock();
	    context_ptr c;
	    if (children.find(f) != children.end())
		c = children[f];
	    lock.unlock();
	    return c;
	}

	// Adds a child context.
	void add_child(const flow& f, context_ptr c) {
	    lock.lock();
	    if (children.find(f) != children.end())
		throw exception("That context already exists.");
	    children[f] = c;
	    lock.unlock();
	}

	// Destructor.
	virtual ~base_context() { 
	    total_contexts--;
	}

	// Returns constructor ID.
	context_id get_id() { return id; }

	// Returns a context 'type'.
	virtual std::string get_type() = 0;

	// Delete myself.
	void delete_myself() {
	    // Erase myself from my parent's child map.
	    // Should call my destructor, I guess.
	    context_ptr p = parent.lock();
	    if (p)
		p->children.erase(addr);
	    else
		throw exception("Could not delete myself.");
	}

    };

};

#endif

