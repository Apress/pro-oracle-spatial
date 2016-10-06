/*
 * @(#)SdoPrint.java 1.0 12-Jul-2003
 *
 * This program uses the Java API for Oracle Spatial supplied with
 * version 10.1 of the Oracle Server (class JGeometry)
 *
 * It illustrates the use of Geometry to extract and process geometry
 * objects stored in tables inside the Oracle database using the
 * SDO_GEOMETRY object type.
 *
 * The program lets you specify connection parameters to a database, as
 * well as the name of a table and the name of a geometry column to
 * process. You can optionally specify a predicate (a WHERE clause) to select
 * the rows to fetch.
 *
 * Finally you can choose the way the geometries will be formatted:
 *
 *  0 = no output.
 *  1 = format as an SDO_GEOMETRY constructor (similar to SQLPLUS output)
 *  2 = display the results from each getXxx() and isXxx() methods of the
 *      Geometry objects
 *
 * You can combine the settings. For example, 3 shows both formats.
 *
 * The program also times the individual steps needed to process a geometry,
 * and displays the total time elapsed for each step:
 *
 * - Getting object = time needed to perform the getObject() method, which
 *   extracts the object from the result set and returns an oracle.sql.STRUCT.
 * - Converting geometry = time to convert the STRUCT into a Geometry object.
 * - Extracting information = time to extract information from the Geometry
 *   object into regular Java variables.
 */

import java.io.*;
import java.sql.*;
import java.util.*;
import java.awt.geom.*;
import java.awt.Shape;
import oracle.jdbc.driver.*;
import oracle.sql.*;
import oracle.spatial.geometry.*;

public final class SdoPrint
{
  public static void main(String args[]) throws Exception
  {
    System.out.println ("SdoPrint - Oracle Spatial (SDO) read");

    // Check and process command line arguments
    if (args.length != 7) {
      System.out.println ("Parameters:");
      System.out.println ("<Connection>:  JDBC connection string");
      System.out.println ("               e.g: jdbc:oracle:thin:@server:port:sid");
      System.out.println ("<User>:        User name");
      System.out.println ("<Password>:    User password");
      System.out.println ("<Table name>:  Table to unload");
      System.out.println ("<Geo column>:  Name of geometry colum,");
      System.out.println ("<Predicate>:   WHERE clause");
      System.out.println ("<Print Style>: 0=none, 1=raw, 2=format");
      return;
    }

    String connectionString  = args[0];
    String userName          = args[1];
    String userPassword      = args[2];
    String tableName         = args[3];
    String geoColumn         = args[4];
    String predicate         = args[5];
    int    printStyle        = Integer.parseInt(args[6]);

    // Register the Oracle JDBC driver
    DriverManager.registerDriver(new oracle.jdbc.driver.OracleDriver());

    // Get a connection to the database
    System.out.println ("Connecting to database '"+connectionString+"'");
    Connection dbConnection = DriverManager.getConnection(connectionString,
      userName, userPassword);
    System.out.println ("Got a connection: "+dbConnection.getClass().getName());

    // Perform the database query
    printGeometries(dbConnection, tableName, geoColumn, predicate, printStyle);

    // Close database connection
    dbConnection.close();
  }

  static void printGeometries(Connection dbConnection, String tableName,
    String geoColumn, String predicate,int printStyle)
    throws Exception
  {
    long totalPoints = 0;
    long totalSize = 0;
    JGeometry geom;

    // Construct SQL query
    String sqlQuery = "SELECT " + geoColumn + " FROM " + tableName + " "
      + predicate;
    System.out.println ("Executing query: '"+sqlQuery+"'");

    // Execute query
    Statement stmt = dbConnection.createStatement();
    OracleResultSet ors = (OracleResultSet) stmt.executeQuery(sqlQuery);

    // Process results
    int rowNumber = 0;
    while (ors.next())
    {
      ++rowNumber;

      // Extract JDBC object from record into structure
      STRUCT dbObject = (STRUCT) ors.getObject(1);

      // Import from structure into Geometry object
      geom = JGeometry.load(dbObject);

      // extract details from jGeometry object
      int gType =               geom.getType();
      int gSRID =               geom.getSRID();
      int gDimensions =         geom.getDimensions();
      long gNumPoints =         geom.getNumPoints();
      long gSize =              geom.getSize();
      boolean isPoint =         geom.isPoint();
      boolean isCircle =        geom.isCircle();
      boolean hasCircularArcs = geom.hasCircularArcs();
      boolean isGeodeticMBR =   geom.isGeodeticMBR();
      boolean isLRSGeometry =   geom.isLRSGeometry();
      boolean isMultiPoint =    geom.isMultiPoint();
      boolean isRectangle =     geom.isRectangle();

      // point
      double gPoint[]  =        geom.getPoint();
      // element info array
      int gElemInfo[] =         geom.getElemInfo();
      int gNumElements =        (gElemInfo == null ? 0 : gElemInfo.length / 3);
      // ordinates array
      double gOrdinates[] =     geom.getOrdinatesArray();

      // other information
      double[] gFirstPoint =    geom.getFirstPoint();
      double[] gLastPoint =     geom.getLastPoint();
      Point2D gLabelPoint =     geom.getLabelPoint();
      Point2D gJavaPoint =      geom.getJavaPoint();
      Point2D[] gJavaPoints =   (isMultiPoint ? geom.getJavaPoints():null);
      double[] gMBR =           geom.getMBR();
      Shape gShape =            geom.createShape();

      totalSize += gSize;
      totalPoints += gNumPoints;

      if (printStyle > 0)
        System.out.println ("Geometry # " + rowNumber + ":");

      // Print out geometry in SDO_GEOMETRY format
      if ((printStyle & 1) == 1 )
        System.out.println (printGeom(geom));

      // Print out formatted geometry
      if ((printStyle & 2) == 2) {
        System.out.println (" Type:            " + gType);
        System.out.println (" SRID:            " + gSRID);
        System.out.println (" Dimensions:      " + gDimensions);
        System.out.println (" NumPoints:       " + gNumPoints);
        System.out.println (" Size:            " + gSize);
        System.out.println (" isPoint:         " + isPoint);
        System.out.println (" isCircle:        " + isCircle);
        System.out.println (" hasCircularArcs: " + hasCircularArcs);
        System.out.println (" isGeodeticMBR:   " + isGeodeticMBR);
        System.out.println (" isLRSGeometry:   " + isLRSGeometry);
        System.out.println (" isMultiPoint:    " + isMultiPoint);
        System.out.println (" isRectangle:     " + isRectangle);
        System.out.println (" MBR:             ("
            + gMBR[0] + " " + gMBR[1] + ") (" + gMBR[2] + " " + gMBR[3] + ") ");
        System.out.println (" First Point:     " + printPoint(gFirstPoint));
        System.out.println (" Last Point:      " + printPoint(gLastPoint));
        System.out.println (" Label Point:     " + printPoint(gLabelPoint));
        System.out.println (" Point:           " + printPoint(gPoint));
        System.out.println (" Java Point:      " + printPoint(gJavaPoint));
        System.out.println (" Java Points List: " +
          (gJavaPoints==null ? "NULL": "["+gJavaPoints.length+"]"));
        if (gJavaPoints != null)
          for (int i=0; i<gJavaPoints.length; i++)
            System.out.println ("   ["+(i+1)+"] (" +
              gJavaPoints[i].getX() + ", " +
              gJavaPoints[i].getY() +")");

        System.out.println (" Elements:        [" + gNumElements + " elements]" );
        if (gElemInfo != null)
          for (int i=0; i<gNumElements; i++)
            System.out.println ("   ["+(i+1)+"] (" +
              gElemInfo[i*3] + ", " +
              gElemInfo[i*3+1] + ", " +
              gElemInfo[i*3+2] + ")");

        System.out.println (" Points List:     [" + gNumPoints + " points]" );
        if (gOrdinates != null)
          for (int i=0; i<gNumPoints; i++) {
            System.out.print ("   ["+(i+1)+"] (");
            for (int j=0; j<gDimensions; j++) {
              System.out.print (gOrdinates[i*gDimensions+j]);
              if (j<gDimensions-1)
                System.out.print (", ");
            }
            System.out.println (")");
          }
      }
    }
    stmt.close();
    System.out.println("");
    System.out.println("Done - "+rowNumber+" geometries extracted");
    System.out.println(" " + totalPoints + " points");
    System.out.println(" " + totalSize + " bytes");
  }

  static String printPoint(double[] point)
  {
    String formattedPoint;
    if (point == null)
      formattedPoint = "NULL";
    else {
      formattedPoint = "["+point.length + "] (";
      for (int i=0; i<point.length; i++) {
        formattedPoint += point[i];
        if (i < point.length-1)
          formattedPoint += ", ";
      }
      formattedPoint += ")";
    }
    return (formattedPoint);
  }

  static String printPoint(Point2D point)
  {
    String formattedPoint;
    if (point == null)
      formattedPoint = "NULL";
    else
      formattedPoint = "[2] (" + point.getX() + ", " + point.getY() + ")";
    return (formattedPoint);
  }

 static String printGeom (JGeometry geom)
  {
    String fg;

    // extract details from jGeometry object
    int gType =               geom.getType();
    int gSRID =               geom.getSRID();
    int gDimensions =         geom.getDimensions();
    boolean isPoint =         geom.isPoint();
    // point
    double gPoint[]  =        geom.getPoint();
    // element info array
    int gElemInfo[] =         geom.getElemInfo();
    // ordinates array
    double gOrdinates[] =     geom.getOrdinatesArray();

    // Format jGeometry in SDO_GEOMETRY format
    int sdo_gtype = gDimensions * 1000 + gType;
    int sdo_srid  = gSRID;

    fg = "SDO_GEOMETRY(" + sdo_gtype + ", ";
    if (sdo_srid == 0)
      fg = fg + "NULL, ";
    else
      fg = fg + sdo_srid + ", ";
    if (gPoint == null)
      fg = fg + "NULL), ";
    else {
      fg = fg + "SDO_POINT_TYPE(" + gPoint[0]+", "+gPoint[1]+", ";
      if (gPoint.length < 3)
        fg = fg + "NULL), ";
      else if (java.lang.Double.isNaN(gPoint[2]))
        fg = fg + "NULL), ";
      else
        fg = fg + gPoint[2]+"), ";
    }
    if (!isPoint & gElemInfo != null) {
      fg = fg + "SDO_ELEM_INFO_ARRAY( ";
      for (int i=0; i<gElemInfo.length-1; i++)
        fg = fg +  gElemInfo[i]+", ";
      fg = fg +  gElemInfo[gElemInfo.length-1] + "), ";
    }
    else
      fg = fg + "NULL, ";
    if (!isPoint & gOrdinates != null) {
      fg = fg + "SDO_ORDINATE_ARRAY( ";
      for (int i=0; i<gOrdinates.length-1; i++)
        fg = fg +  gOrdinates[i]+", ";
      fg = fg +  gOrdinates[gOrdinates.length-1] + ")";
    }
    else
      fg = fg + "NULL";
    fg = fg + ")";

    return (fg);
  }
}
